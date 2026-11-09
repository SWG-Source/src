// ======================================================================
//
// snapshot_send_test.cpp
//
// SwgSnapshot with the generated encoders, built from the SwgDatabaseServer
// sources: a value that does not fit its member rejects the whole snapshot
// before anything is sent, and a NULL rank or posture encodes as the
// new-creature default. The creature rows are read from a scratch schema
// inside a transaction that is rolled back. See README.md.
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "SwgDatabaseServer/SwgSnapshot.h"
#include "serverGame/ServerCreatureObjectTemplate.h"
#include "sharedDatabaseInterface/DbServer.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedNetworkMessages/BatchBaselinesMessage.h"

#include "TestConnection.h"

#include <cstdio>

static int s_failures = 0;
static void check(bool ok, char const *what) { printf("%s: %s\n", ok ? "PASS" : "FAIL", what); if (!ok) ++s_failures; }

class TestSnapshot : public SwgSnapshot
{
public:
	TestSnapshot() : SwgSnapshot(DB::ModeQuery::mode_UPDATE, false) {}
	DBSchema::CreatureObjectBufferRow *creature(NetworkId const &id) { return m_creatureObjectBuffer.findRowByIndex(id); }
	size_t preparedBaselines() const { return m_preparedBaselines.size(); }
	std::vector<unsigned char> encodedBaselines() const
	{
		// Match ServerConnection::send: constructing a message does not pack
		// its AutoVariables into GameNetworkMessage::getByteStream().
		BatchBaselinesMessage const message(m_preparedBaselines);
		Archive::ByteStream bytes;
		message.pack(bytes);
		return std::vector<unsigned char>(bytes.getBuffer(), bytes.getBuffer() + bytes.getSize());
	}
	size_t preparedObjects() const { size_t n = 0; for (size_t i = 0; i < m_preparedObjects.size(); ++i) if (m_preparedObjects[i] != NetworkId::cms_invalid) ++n; return n; }
};

class SqlQuery : public DB::Query
{
public:
	explicit SqlQuery(std::string const &sql) : m_sql(sql) {}
	virtual void getSQL(std::string &sql) { sql = m_sql; }
	virtual bool bindParameters() { return true; }
	virtual bool bindColumns() { return true; }
	virtual QueryMode getExecutionMode() const { return MODE_DML; }
private:
	std::string m_sql;
};

class CreatureQuery : public DB::Query
{
public:
	virtual void getSQL(std::string &sql) { sql = "select object_id, posture, rank from creature_objects where object_id between 770000201 and 770000205 order by object_id"; }
	virtual bool bindParameters() { return true; }
	virtual bool bindColumns() { return bindCol(object_id) && bindCol(posture) && bindCol(rank); }
	virtual QueryMode getExecutionMode() const { return MODE_SQL; }
	DB::BindableNetworkId object_id;
	DB::BindableInt32 posture, rank;
};

static bool run(DB::Session *s, char const *sql) { SqlQuery q(sql); bool ok = s->exec(&q); q.done(); return ok; }

int main()
{
	DB::Server::setFatalOnError(false);
	Int32TestConnection::Parameters const parameters = Int32TestConnection::fromEnvironment();
	DB::Server *server = DB::Server::create(parameters.dsn.c_str(), parameters.user.c_str(), parameters.password.c_str(), DB::PROTOCOL_OCI, false);
	DB::Session *session = server->getSession();
	check(session != nullptr, "connected to the scratch schema");
	if (!session)
		return 1;
	session->setAutoCommitMode(false);
	check(run(session, "insert into creature_objects (object_id, posture, rank) values (770000201, 0, 3)")
		&& run(session, "insert into creature_objects (object_id, posture, rank) values (770000202, 1, 256)")
		&& run(session, "insert into creature_objects (object_id, posture, rank) values (770000203, 128, 2)")
		&& run(session, "insert into creature_objects (object_id, posture, rank) values (770000204, null, null)")
		&& run(session, "insert into creature_objects (object_id, posture, rank) values (770000205, 0, 0)"),
		"seed creatures (0,3), (1,256), (128,2), (NULL,NULL), (0,0)");

	struct Loaded { NetworkId id; DB::BindableInt32 posture, rank; };
	std::vector<Loaded> rows;
	{
		CreatureQuery q;
		check(session->exec(&q), "select seeded creatures");
		while (q.fetch() > 0) { Loaded l; l.id = q.object_id.getValue(); l.posture = q.posture; l.rank = q.rank; rows.push_back(l); }
		q.done();
	}
	check(session->rollbackTransaction(), "rollback: the scratch schema is unchanged");
	check(rows.size() == 5, "five creature rows");
	if (rows.size() != 5) return 1;

	Tag const creatureTag = ServerCreatureObjectTemplate::ServerCreatureObjectTemplate_tag;

	// good creature alone: prepares, and send() only fails for want of a connection
	{
		TestSnapshot s;
		s.newObject(rows[0].id, 0x12345678u, creatureTag);
		s.creature(rows[0].id)->posture = rows[0].posture;
		s.creature(rows[0].id)->rank = rows[0].rank;
		check(s.prepareSend(), "valid creature: prepareSend succeeds");
		check(s.preparedObjects() == 1 && s.preparedBaselines() > 0, "valid creature: baselines encoded");
	}
	// a snapshot with a good creature, then one whose rank (256) or posture (128)
	// does not fit: the whole snapshot is rejected and nothing encoded is kept
	for (size_t bad = 1; bad <= 2; ++bad)
	{
		TestSnapshot s;
		s.newObject(rows[0].id, 0x12345678u, creatureTag);
		s.creature(rows[0].id)->posture = rows[0].posture;
		s.creature(rows[0].id)->rank = rows[0].rank;
		s.newObject(rows[bad].id, 0x12345678u, creatureTag);
		s.creature(rows[bad].id)->posture = rows[bad].posture;
		s.creature(rows[bad].id)->rank = rows[bad].rank;
		bool const prepared = s.prepareSend();
		printf("  object %s: prepared=%d baselines=%zu objects=%zu\n", rows[bad].id.getValueString().c_str(), prepared ? 1 : 0, s.preparedBaselines(), s.preparedObjects());
		check(!prepared, bad == 1 ? "rank 256: prepareSend rejects the snapshot" : "posture 128: prepareSend rejects the snapshot");
		check(s.preparedBaselines() == 0 && s.preparedObjects() == 0, "rejected snapshot keeps no partial output (the valid creature's baselines are discarded too)");
		check(!s.prepareSend(), "rejection is sticky");
		check(!s.send(nullptr), "send() refuses a rejected snapshot before touching any connection");
	}

	// NULL rank and posture encode exactly as a creature with posture Upright
	// (0) and rank 0, the values a new CreatureObject gets
	{
		TestSnapshot nulls;
		nulls.newObject(rows[3].id, 0x12345678u, creatureTag);
		nulls.creature(rows[3].id)->posture = rows[3].posture;
		nulls.creature(rows[3].id)->rank = rows[3].rank;
		TestSnapshot zeros;
		zeros.newObject(rows[3].id, 0x12345678u, creatureTag);
		zeros.creature(rows[3].id)->posture = rows[4].posture;
		zeros.creature(rows[3].id)->rank = rows[4].rank;
		check(rows[3].rank.isNull() && rows[3].posture.isNull(), "the NULL creature reads NULL rank and posture");
		check(nulls.prepareSend() && zeros.prepareSend(), "NULL rank/posture: prepareSend succeeds (with a warning for each)");
		std::vector<unsigned char> const n = nulls.encodedBaselines();
		std::vector<unsigned char> const z = zeros.encodedBaselines();
		size_t firstDifference = 0;
		while (firstDifference < n.size() && firstDifference < z.size() && n[firstDifference] == z[firstDifference])
			++firstDifference;
		if (n != z)
			printf("  encoded sizes %zu and %zu, first difference at byte %zu\n", n.size(), z.size(), firstDifference);
		check(n == z && !n.empty(),
			"NULL rank/posture encode byte for byte as posture Upright (0), rank 0");

		// Sensitivity control: the same object with rank 1 must produce
		// different bytes. This catches an empty or unrelated measurement.
		TestSnapshot rankOne;
		rankOne.newObject(rows[3].id, 0x12345678u, creatureTag);
		rankOne.creature(rows[3].id)->posture = rows[4].posture;
		rankOne.creature(rows[3].id)->rank.setValue(int32_t(1));
		check(rankOne.prepareSend(), "rank 1 sensitivity control: prepareSend succeeds");
		check(rankOne.encodedBaselines() != z,
			"rank 1 sensitivity control: changing only rank changes the serialized message");
	}

	server->releaseSession(session);
	server->disconnect();
	printf("%s: %d failure(s)\n", s_failures == 0 ? "ALL PASSED" : "FAILED", s_failures);
	return s_failures == 0 ? 0 : 1;
}
