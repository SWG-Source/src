// ======================================================================
//
// BattlefieldParticipantQuery.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_BattlefieldParticipantQuery_H
#define INCLUDED_BattlefieldParticipantQuery_H

// ======================================================================

#include "sharedDatabaseInterface/DbModeQuery.h"
#include "SwgDatabaseServer/Schema.h"
#include <vector>

// ======================================================================

namespace DBQuery
{
	
	class BattlefieldParticipantQuery : public DB::ModeQuery
	{
		BattlefieldParticipantQuery(const BattlefieldParticipantQuery&); // disable
		BattlefieldParticipantQuery& operator=(const BattlefieldParticipantQuery&); // disable
	  public:
		BattlefieldParticipantQuery();
		
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual void getSQL(std::string &sql);
	};

// ----------------------------------------------------------------------

	class GetAllBattlefieldParticipant : public DB::Query
	{
	  public:
		GetAllBattlefieldParticipant(const std::string &schema);
		
		const std::vector<DBSchema::BattlefieldParticipantRow> &getData() const;
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual void getSQL(std::string &sql);
		virtual QueryMode getExecutionMode() const;

	  private:
		GetAllBattlefieldParticipant(const GetAllBattlefieldParticipant&); // disable
		GetAllBattlefieldParticipant& operator=(const GetAllBattlefieldParticipant&); // disable

	  private:
		std::vector<DBSchema::BattlefieldParticipantRow> m_data;
		const std::string m_schema;
	};
}

// ======================================================================

inline const std::vector<DBSchema::BattlefieldParticipantRow> &DBQuery::GetAllBattlefieldParticipant::getData() const
{
	return m_data;
}

// ======================================================================

#endif
