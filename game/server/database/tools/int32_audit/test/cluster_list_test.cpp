// ======================================================================
//
// cluster_list_test.cpp
//
// TaskGetClusterList, built from the LoginServer sources: a cluster whose
// port does not fit uint16 is skipped with a logged error, and the other
// clusters are still served. The cluster rows are seeded in a scratch
// schema inside a transaction that is rolled back. See README.md.
//
// The test sets ConfigLoginServer's data directly instead of installing
// the LoginServer's configuration; that is why it opens up the class.
//
// ======================================================================
#include "FirstLoginServer.h"
#define private public
#include "ConfigLoginServer.h"
#include "TaskGetClusterList.h"
#undef private
#include "sharedDatabaseInterface/DbServer.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "TestConnection.h"

#include <cstdio>
#include <cstring>

static int s_failures = 0;
static void check(bool ok, char const *what) { printf("%s: %s\n", ok ? "PASS" : "FAIL", what); if (!ok) ++s_failures; }

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
static bool run(DB::Session *s, char const *sql) { SqlQuery q(sql); bool ok = s->exec(&q); q.done(); return ok; }

int main()
{
	static ConfigLoginServer::Data data;
	memset(&data, 0, sizeof(data));
	data.schemaOwner = "";
	data.centralServicePort = 44463;
	ConfigLoginServer::data = &data;

	DB::Server::setFatalOnError(false);
	Int32TestConnection::Parameters const parameters = Int32TestConnection::fromEnvironment();
	DB::Server *server = DB::Server::create(parameters.dsn.c_str(), parameters.user.c_str(), parameters.password.c_str(), DB::PROTOCOL_OCI, false);
	DB::Session *session = server->getSession();
	check(session != nullptr, "connected to the scratch schema");
	if (!session)
		return 1;
	session->setAutoCommitMode(false);
	check(run(session, "insert into cluster_list (id, name, address, port, group_id) values (91, 'int32test-a', 'a', 44463, 1)")
		&& run(session, "insert into cluster_list (id, name, address, port, group_id) values (92, 'int32test-bad', 'b', 65536, 1)")
		&& run(session, "insert into cluster_list (id, name, address, port, group_id) values (93, 'int32test-c', 'c', null, 1)"),
		"seed clusters 91 (port 44463), 92 (port 65536), 93 (NULL port)");

	TaskGetClusterList task(1);
	bool const ok = task.process(session);
	check(ok, "TaskGetClusterList::process succeeds");
	std::string names;
	bool sawBad = false;
	uint16 portA = 0, portC = 0;
	for (size_t i = 0; i < task.m_clusterData.size(); ++i)
	{
		TaskGetClusterList::ClusterData const &c = task.m_clusterData[i];
		names += c.m_clusterName + " ";
		if (c.m_clusterId == 92) sawBad = true;
		if (c.m_clusterId == 91) portA = c.m_port;
		if (c.m_clusterId == 93) portC = c.m_port;
	}
	printf("  clusters served: %s\n", names.c_str());
	check(!sawBad, "cluster 92 (port 65536) is skipped");
	check(portA == 44463, "cluster 91 is served with port 44463");
	check(portC == 44463, "cluster 93 (NULL port) is served with the configured central service port");
	check(session->rollbackTransaction(), "rollback: the scratch schema is unchanged");
	server->releaseSession(session);
	server->disconnect();
	printf("%s: %d failure(s)\n", s_failures == 0 ? "ALL PASSED" : "FAILED", s_failures);
	return s_failures == 0 ? 0 : 1;
}
