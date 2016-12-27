// ======================================================================
//
// TaskMapAccount.cpp
// copyright (c) 2016 StellaBellum
//
// ======================================================================

#include "FirstLoginServer.h"
#include "TaskMapAccount.h"

#include "ClientConnection.h"
#include "DatabaseConnection.h"
#include "sharedDatabaseInterface/DbSession.h"

// ======================================================================

TaskMapAccount::TaskMapAccount(StationId parentID, StationId childID) :
        TaskRequest(),
        m_parentID(parentID),
        m_childID(childID) {
}

// ----------------------------------------------------------------------

bool TaskMapAccount::process(DB::Session *session) {
    MapAccountQuery qry;
    qry.parentID = m_parentID;
    qry.childID = m_childID;

    bool rval = session->exec(&qry);

    qry.done();
    return rval;
}

// ----------------------------------------------------------------------

void TaskMapAccount::onComplete() {
}

// ======================================================================

TaskMapAccount::MapAccountQuery::MapAccountQuery() :
        Query(),
        parentID(),
        childID() {
}

// ----------------------------------------------------------------------

void TaskMapAccount::MapAccountQuery::getSQL(std::string &sql) {
    sql = std::string("begin ") + DatabaseConnection::getInstance().getSchemaQualifier() +
          "merge into account_map am using (select :parentID parent_id, :childID child_id from dual) v on (am.parent_id = v.parent_id and am.child_id = v.child_id) when not matched then insert (parent_id, child_id) values (v.parent_id, v.child_id); end;";
}

// ----------------------------------------------------------------------

bool TaskMapAccount::MapAccountQuery::bindParameters() {
    if (!bindParameter(parentID)) return false;
    if (!bindParameter(childID)) return false;
    return true;
}

// ----------------------------------------------------------------------

bool TaskMapAccount::MapAccountQuery::bindColumns() {
    return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode TaskMapAccount::MapAccountQuery::getExecutionMode() const {
    return MODE_PROCEXEC;
}

// ======================================================================
