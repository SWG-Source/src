// ======================================================================
//
// db_binding_test.cpp
//
// Query-level tests of the 32-bit database bindings, against a scratch
// schema built with the normal database scripts. See README.md.
//
// Everything the test seeds is written in one transaction that is rolled
// back at the end. The only DDL is a scratch table and a scratch function
// (int32test_*), created before the transaction and dropped at the end.
//
// ======================================================================

#include "sharedDatabaseInterface/FirstSharedDatabaseInterface.h"
#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbBindableVarray.h"
#include "sharedDatabaseInterface/DbCheckedConversion.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbServer.h"
#include "sharedDatabaseInterface/DbSession.h"

#include "TestConnection.h"

#include <cstdio>
#include <type_traits>
#include <utility>
#include <cstdint>
#include <string>
#include <vector>

static int s_failures = 0;
static void check(bool ok, char const *what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		++s_failures;
}

// ----------------------------------------------------------------------

class SqlQuery : public DB::Query
{
public:
	explicit SqlQuery(std::string const &sql, QueryMode mode = MODE_DML) : m_sql(sql), m_mode(mode) {}
	virtual void getSQL(std::string &sql) { sql = m_sql; }
	virtual bool bindParameters() { return true; }
	virtual bool bindColumns() { return true; }
	virtual QueryMode getExecutionMode() const { return m_mode; }
private:
	std::string m_sql;
	QueryMode m_mode;
};

static bool run(DB::Session *session, std::string const &sql)
{
	SqlQuery q(sql);
	bool const ok = session->exec(&q);
	q.done();
	return ok;
}

// ----------------------------------------------------------------------
// The shape of ObjectsTableQuerySelect: loader.load_object's ref cursor,
// fetched in array mode.

struct LoadRow
{
	DB::BindableNetworkId object_id;
	DB::BindableDouble x, y, z, qw, qx, qy, qz;
	DB::BindableInt32 node_x, node_y, node_z;
	DB::BindableUint32 object_template_id;
};

class LoadObjectQuery : public DB::Query
{
public:
	explicit LoadObjectQuery(size_t batch) : m_data(batch), m_batch(batch) {}
	virtual void getSQL(std::string &sql) { sql = "begin :result := loader.load_object; end;"; }
	virtual bool bindParameters() { return true; }
	virtual bool bindColumns()
	{
		size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
		setColArrayMode(skipSize, m_batch);
		LoadRow &r = m_data[0];
		return bindCol(r.object_id) && bindCol(r.x) && bindCol(r.y) && bindCol(r.z) && bindCol(r.qw) && bindCol(r.qx)
			&& bindCol(r.qy) && bindCol(r.qz) && bindCol(r.node_x) && bindCol(r.node_y) && bindCol(r.node_z) && bindCol(r.object_template_id);
	}
	virtual QueryMode getExecutionMode() const { return MODE_PLSQL_REFCURSOR; }
	std::vector<LoadRow> m_data;
private:
	size_t m_batch;
};

// ----------------------------------------------------------------------
// resource_types.depleted_timestamp in array mode (batch of 2): a NULL
// fetched into a row that held a value in the previous batch.

struct DepletedRow { DB::BindableNetworkId id; DB::BindableUint32 ts; DB::BindableDouble d; DB::BindableBool b; };
class DepletedQuery : public DB::Query
{
public:
	DepletedQuery() : m_data(2) {}
	virtual void getSQL(std::string &sql) { sql = "begin :result := int32test_null_cursor; end;"; }
	virtual bool bindParameters() { return true; }
	virtual bool bindColumns()
	{
		size_t skip = reinterpret_cast<char*>(&m_data[1]) - reinterpret_cast<char*>(&m_data[0]);
		setColArrayMode(skip, 2);
		return bindCol(m_data[0].id) && bindCol(m_data[0].ts) && bindCol(m_data[0].d) && bindCol(m_data[0].b);
	}
	virtual QueryMode getExecutionMode() const { return MODE_PLSQL_REFCURSOR; }
	std::vector<DepletedRow> m_data;
};

// ----------------------------------------------------------------------

class VarrayInsertQuery : public DB::Query
{
public:
	virtual void getSQL(std::string &sql) { sql = "begin insert into int32test_values (seq, v) select rownum, column_value from table(:arr); end;"; }
	virtual bool bindParameters() { return bindParameter(m_values); }
	virtual bool bindColumns() { return true; }
	virtual QueryMode getExecutionMode() const { return MODE_PROCEXEC; }
	DB::BindableVarrayNumber m_values;
};

class ReadCrcQuery : public DB::Query
{
public:
	virtual void getSQL(std::string &sql) { sql = "select seq, case when seq <> 4 then v end, case when seq <> 4 then v end, nvl(to_char(v), 'NULL') from int32test_values order by seq"; } // row 4 holds 2^40, which no 32-bit column can read
	virtual bool bindParameters() { return true; }
	virtual bool bindColumns() { return bindCol(seq) && bindCol(asInt32) && bindCol(asUint32) && bindCol(text); }
	virtual QueryMode getExecutionMode() const { return MODE_SQL; }
	DB::BindableInt32 seq;
	DB::BindableInt32 asInt32;
	DB::BindableUint32 asUint32;
	DB::BindableString<40> text;
};

class ParamInsertQuery : public DB::Query
{
public:
	virtual void getSQL(std::string &sql) { sql = "insert into int32test_values (seq, v) values (:seq, :v)"; }
	virtual bool bindParameters() { return bindParameter(seq) && bindParameter(crc); }
	virtual bool bindColumns() { return true; }
	virtual QueryMode getExecutionMode() const { return MODE_DML; }
	DB::BindableInt32 seq;
	DB::BindableUint32 crc;
};

class ReturnParamQuery : public DB::Query
{
public:
	virtual void getSQL(std::string &sql) { sql = "begin :n := :a + 1; :s := :t || 'x'; end;"; }
	virtual bool bindParameters() { return bindParameter(n) && bindParameter(a) && bindParameter(s) && bindParameter(t); }
	virtual bool bindColumns() { return true; }
	virtual QueryMode getExecutionMode() const { return MODE_PROCEXEC; }
	DB::BindableInt32 n, a;
	DB::BindableString<20> s, t;
};

class NullQuery : public DB::Query
{
public:
	virtual void getSQL(std::string &sql) { sql = "select cast(null as number) from dual"; }
	virtual bool bindParameters() { return true; }
	virtual bool bindColumns() { return bindCol(value); }
	virtual QueryMode getExecutionMode() const { return MODE_SQL; }
	DB::BindableInt32 value;
};

// ----------------------------------------------------------------------
// The varray has fixed-width integer overloads only: a platform-width or
// unsigned argument has no single best overload and does not compile.

template <typename T, typename = void> struct CanPush : std::false_type {};
template <typename T> struct CanPush<T, decltype(void(std::declval<DB::BindableVarrayNumber &>().push_back(std::declval<T>())))> : std::true_type {};
static_assert(CanPush<int32_t>::value && CanPush<int64_t>::value, "fixed-width pushes compile");
static_assert(!CanPush<unsigned int>::value && !CanPush<uint32_t>::value && !CanPush<unsigned long>::value, "unsigned pushes do not compile");
static_assert(sizeof(long) == 8 ? !CanPush<long long>::value : !CanPush<long>::value, "the type that is not int64_t on this ABI does not compile");

// ----------------------------------------------------------------------

static void checkedConversionTests()
{
	uint8_t u8 = 7;
	check(!DB::checkedNarrow(int32_t(300), u8, "unit uint8 300", "harness") && u8 == 7, "checkedNarrow: 300 -> uint8 fails, destination unchanged");
	check(!DB::checkedNarrow(int32_t(-1), u8, "unit uint8 -1", "harness") && u8 == 7, "checkedNarrow: -1 -> uint8 fails");
	check(DB::checkedNarrow(int32_t(255), u8, "unit", "harness") && u8 == 255, "checkedNarrow: 255 -> uint8");
	int8_t i8 = 0;
	check(!DB::checkedNarrow(int32_t(-999), i8, "unit int8 -999 (a NULL column's sentinel)", "harness"), "checkedNarrow: -999 -> int8 fails");
	unsigned int ub4v = 0;
	check(!DB::checkedNarrow(static_cast<unsigned long long>(1) << 32, ub4v, "unit ub4 2^32", "harness"), "checkedNarrow: 2^32 -> ub4 fails");
	check(DB::checkedNarrow(size_t(4294967295u), ub4v, "unit", "harness") && ub4v == 4294967295u, "checkedNarrow: 2^32-1 -> ub4");
	unsigned short ub2v = 0;
	check(!DB::checkedNarrow(int(65536), ub2v, "unit ub2 65536", "harness"), "checkedNarrow: 65536 -> ub2 fails");
	check(DB::checkedNarrow(int(65535), ub2v, "unit", "harness") && ub2v == 65535, "checkedNarrow: 65535 -> ub2");
	int iv = 0;
	check(!DB::checkedNarrow(int64_t(2147483647) + 7 * 60 * 60, iv, "unit time + 7h", "harness"), "checkedNarrow: INT_MAX + 7h -> int fails");
	check(!DB::checkedNarrow(unsigned(2147483648u), iv, "unit", [](){ return std::string("lazy context"); }), "checkedNarrow: 2^31 -> int fails (lazy context)");
	check(DB::checkedNarrow(int64_t(-2147483647 - 1), iv, "unit", "harness") && iv == -2147483647 - 1, "checkedNarrow: INT_MIN -> int");
	size_t idx = 0;
	check(!DB::checkedNarrow(int32_t(-2), idx, "unit size_t -2", "harness"), "checkedNarrow: -2 -> size_t fails");
	DB::BindableInt32 fresh;
	check(fresh.isNull() && fresh.getValue() == -999, "BindableInt32 default is NULL (sentinel -999, not 0)");
	fresh = 5;
	check(!fresh.isNull() && fresh.getValue() == 5, "BindableInt32 set");
	fresh.setNull();
	check(fresh.isNull(), "BindableInt32 setNull");
	check(sizeof(int32_t) == 4, "int32_t is 4 bytes");
}

// ----------------------------------------------------------------------

int main()
{
	printf("ABI: %d-bit (sizeof(long)=%d)\n", static_cast<int>(sizeof(void*) * 8), static_cast<int>(sizeof(long)));
	checkedConversionTests();

	Int32TestConnection::Parameters const parameters = Int32TestConnection::fromEnvironment();
	DB::Server::setFatalOnError(false);
	DB::Server *server = DB::Server::create(parameters.dsn.c_str(), parameters.user.c_str(), parameters.password.c_str(), DB::PROTOCOL_OCI, false);
	DB::Session *session = server->getSession();
	check(session != nullptr, "connected to the scratch schema");
	if (!session)
		return 1;
	session->setFatalOnError(false);

	// --- scratch table (DDL commits, so before the transaction) ---
	run(session, "begin execute immediate 'drop table int32test_values'; exception when others then null; end;");
	check(run(session, "create table int32test_values (seq number, v number)"), "create scratch table int32test_values");
	check(run(session, "create or replace function int32test_null_cursor return sys_refcursor as c sys_refcursor; begin open c for "
		"select resource_id, depleted_timestamp, case when depleted_timestamp is null then null else 1.5 end, case when depleted_timestamp is null then null else 'Y' end "
		"from resource_types where resource_id between 770000101 and 770000103 order by resource_id; return c; end;"), "create the scratch cursor function");


	// --- varray adapters: each column pushed at its own width, NULL as NULL ---
	{
		VarrayInsertQuery q;
		check(q.m_values.create(session, "VAOFNUMBER", parameters.schemaOwner()), "create VAOFNUMBER varray");
		DB::BindableUint32 crc(2404701341u);
		DB::BindableInt32 nullInt;
		DB::BindableInt32 negative(-5);
		DB::BindableInt64 wide(int64(1) << 40);
		DB::BindableUint32 nullCrc;
		check(q.m_values.push_back(crc), "push BindableUint32 2404701341");
		check(q.m_values.push_back(nullInt), "push NULL BindableInt32");
		check(q.m_values.push_back(negative), "push BindableInt32 -5");
		check(q.m_values.push_back(wide), "push BindableInt64 2^40");
		check(q.m_values.push_back(nullCrc.isNull(), nullCrc), "push NULL BindableUint32 (IsNULL form)");
		DB::BindableDouble nullDouble;
		DB::BindableUint32 callTime(0x80000000u);
		check(q.m_values.push_back(nullDouble), "push NULL BindableDouble (no value read, no conversion)");
		check(q.m_values.push_back(callTime), "push BindableUint32 call time 0x80000000");
		check(session->exec(&q), "insert varray through table(:arr)");
		q.done();
		q.m_values.free();
	}
	// --- scalar parameter bind of a CRC ---
	{
		ParamInsertQuery q;
		q.seq = 8;
		q.crc = 4294967295u;
		check(session->exec(&q), "insert BindableUint32 4294967295 as a parameter");
		check(q.rowCount() == 1, "rowCount() of that insert is 1");
		q.done();
	}
	{
		ReadCrcQuery q;
		check(session->exec(&q), "select int32test_values");
		std::vector<std::string> text;
		std::vector<bool> nulls;
		std::vector<uint32_t> asUnsigned;
		std::vector<int32_t> asSigned;
		while (q.fetch() > 0)
		{
			text.push_back(q.text.getValueASCII());
			nulls.push_back(q.asInt32.isNull());
			asUnsigned.push_back(q.asUint32.getValue());
			asSigned.push_back(q.asInt32.getValue());
		}
		q.done();
		check(text.size() == 8, "eight rows stored");
		if (text.size() == 8)
		{
			printf("  stored: %s | %s | %s | %s | %s | %s | %s | %s\n", text[0].c_str(), text[1].c_str(), text[2].c_str(), text[3].c_str(), text[4].c_str(), text[5].c_str(), text[6].c_str(), text[7].c_str());
			check(text[5] == "NULL", "NULL BindableDouble element stored as NULL");
			check(text[6] == "-2147483648" && asUnsigned[6] == 0x80000000u, "call time 0x80000000 stored as -2147483648, read back bit-exactly as 2147483648");
			check(text[0] == "-1890265955" && asUnsigned[0] == 2404701341u, "CRC 2404701341 stored as -1890265955 and read back as 2404701341");
			check(text[1] == "NULL" && nulls[1], "NULL BindableInt32 element stored as NULL (not 0, not -999)");
			check(text[2] == "-5" && asSigned[2] == -5, "BindableInt32 -5 round-trips");
			check(text[3] == "1099511627776", "BindableInt64 2^40 stored at 64 bits");
			check(text[4] == "NULL", "NULL BindableUint32 element stored as NULL");
			check(text[7] == "-1" && asUnsigned[7] == 4294967295u, "CRC 4294967295 parameter stored as -1, read back as 4294967295");
		}
	}
	{
		ReturnParamQuery q;
		q.a = 41;
		q.t = std::string("abc");
		q.n.setNull();
		q.s.setNull();
		check(session->exec(&q), "PL/SQL in/out int and string parameters (bind lengths through preprocessBinds)");
		check(q.n.getValue() == 42 && q.s.getValueASCII() == "abcx", "out parameters: 42, \"abcx\"");
		q.done();
	}
	{
		NullQuery q;
		check(session->exec(&q) && q.fetch() == 1, "select NULL into BindableInt32");
		check(q.value.isNull(), "NULL column fetched as NULL");
		q.done();
	}
	run(session, "drop table int32test_values");

	// --- the object load path, in one transaction that is rolled back ---
	check(session->setAutoCommitMode(false), "manual commit mode");
	check(run(session, "insert into objects (object_id, node_x, node_y, node_z) select 770000000 + level, level, 1, 2 from dual connect by level <= 10"), "seed objects 770000001..770000010");
	check(run(session, "update objects set object_template_id = -1890265955, node_x = 7, node_y = 8, node_z = 9 where object_id = 770000002"), "give object 770000002 a negative template CRC");
	check(run(session, "insert into object_list (object_id) select object_id from objects where object_id between 770000001 and 770000010"), "fill object_list with the 10 objects");

	{
		LoadObjectQuery q(4); // 10 objects: batches of 4, 4 and a short final 2
		check(session->exec(&q), "exec loader.load_object");
		std::vector<int> batches;
		bool crcOk = false;
		bool staleUntouched = false;
		int rows;
		while ((rows = q.fetch()) > 0)
		{
			batches.push_back(rows);
			for (int i = 0; i < rows; ++i)
			{
				if (q.m_data[i].object_id.getValue() == NetworkId(static_cast<NetworkId::NetworkIdType>(770000002)))
					crcOk = (q.m_data[i].object_template_id.getValue() == 2404701341u && q.m_data[i].node_x.getValue() == 7);
			}
			if (batches.size() == 3)
			{
				// rows 2 and 3 of the batch were nulled after batch 2, and the
				// short final batch must not revive them from stale indicators
				staleUntouched = q.m_data[2].node_x.isNull() && q.m_data[3].node_x.isNull()
					&& q.m_data[2].object_template_id.isNull() && q.m_data[3].object_template_id.isNull()
					&& !q.m_data[0].node_x.isNull() && !q.m_data[1].node_x.isNull();
			}
			for (size_t i = 0; i < q.m_data.size(); ++i)
			{
				q.m_data[i].node_x.setNull();
				q.m_data[i].object_template_id.setNull();
			}
		}
		check(rows == 0, "load finished without error");
		printf("  batches:");
		for (size_t i = 0; i < batches.size(); ++i)
			printf(" %d", batches[i]);
		printf("\n");
		check(batches.size() == 3 && batches[0] == 4 && batches[1] == 4 && batches[2] == 2, "fetch returned 4, 4, 2");
		check(crcOk, "stored template CRC -1890265955 loads as uint32 2404701341");
		check(staleUntouched, "short final batch post-processed only its 2 rows");
		q.done();
	}

	// --- an int column outside int32 fails the load; nothing is clamped ---
	check(run(session, "update objects set node_x = 3000000000 where object_id = 770000007"), "seed node_x = 3000000000 on object 770000007");
	check(run(session, "delete from object_list") && run(session, "insert into object_list (object_id) select object_id from objects where object_id between 770000001 and 770000010"), "refill object_list");
	{
		LoadObjectQuery q(4);
		bool execOk = session->exec(&q);
		int rows = execOk ? 0 : -1;
		bool clamped = false;
		int total = 0;
		if (execOk)
		{
			while ((rows = q.fetch()) > 0)
			{
				total += rows;
				for (int i = 0; i < rows; ++i)
				{
					int32_t const v = q.m_data[i].node_x.getValue();
					if (v == 2147483647 || v == -1294967296 || v == -2147483647 - 1)
						clamped = true;
				}
			}
		}
		printf("  exec=%d, last fetch=%d, rows delivered before the failure=%d\n", execOk ? 1 : 0, rows, total);
		check(rows < 0, "load of the out-of-range row fails (fetch returns -1, ORA-01455 logged above)");
		check(!clamped, "no clamped or wrapped value was delivered");
		q.done();
	}

	// --- a NULL fetched into a reused row must not inherit the previous batch's value ---
	check(run(session, "insert into resource_types (resource_id, resource_name, depleted_timestamp) values (770000101, 'int32test_a', 5)")
		&& run(session, "insert into resource_types (resource_id, resource_name, depleted_timestamp) values (770000102, 'int32test_b', 7)")
		&& run(session, "insert into resource_types (resource_id, resource_name, depleted_timestamp) values (770000103, 'int32test_c', null)"),
		"seed resource types 770000101 (5), 770000102 (7), 770000103 (NULL)");
	{
		DepletedQuery q;
		check(session->exec(&q), "exec depleted-timestamp cursor");
		int rows;
		int batch = 0;
		bool inherited = true;
		while ((rows = q.fetch()) > 0)
		{
			++batch;
			if (batch == 1)
				check(rows == 2 && q.m_data[0].ts.getValue() == 5 && q.m_data[1].ts.getValue() == 7, "batch 1: 5 and 7");
			if (batch == 2)
			{
				DepletedRow const &r = q.m_data[0];
				printf("  batch 2 row 0: isNull=%d uint32=%u double=%g bool=%d\n", r.ts.isNull() ? 1 : 0, r.ts.getValue(), r.d.getValue(), r.b.getValue() ? 1 : 0);
				inherited = !(rows == 1 && r.ts.isNull() && r.ts.getValue() == 0 && r.d.getValue() == -999.0 && !r.b.getValue());
			}
		}
		check(batch == 2 && !inherited, "NULL depleted_timestamp in a reused row reads NULL with the type's null value, not the previous batch's 5 (double and bool too)");
		q.done();
	}
	check(session->rollbackTransaction(), "rollback: the scratch schema is unchanged");
	run(session, "drop function int32test_null_cursor");
	session->setAutoCommitMode(true);
	server->releaseSession(session);
	server->disconnect();
	delete server;

	printf("%s: %d failure(s)\n", s_failures == 0 ? "ALL PASSED" : "FAILED", s_failures);
	return s_failures == 0 ? 0 : 1;
}
