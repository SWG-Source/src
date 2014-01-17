// ======================================================================
//
// CharacterLocator.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CharacterLocator_H
#define INCLUDED_CharacterLocator_H

#include "serverDatabase/ObjectLocator.h"
#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedFoundation/NetworkId.h"

// ======================================================================

/**
 * Locate a specified character and add it to the list of objects to be
 * loaded by the Snapshot.
 */
class CharacterLocator : public ObjectLocator
{
  public:
	CharacterLocator(const NetworkId &characterId);
	virtual ~CharacterLocator();
	
  public:
	bool locateObjects(DB::Session *session, const std::string &schema, int &objectsLocated);
	
  private:
	NetworkId m_characterId;

  private:
	class LocateCharacterQuery : public DB::Query
	{
	  public:
		LocateCharacterQuery(const NetworkId &networkId, const std::string &schema);

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();

	  private:
		virtual DB::Query::QueryMode getExecutionMode() const;
		
	  private:
		LocateCharacterQuery(const LocateCharacterQuery&); // disable
		LocateCharacterQuery& operator=(const LocateCharacterQuery&); //disable

	  public:
		DB::BindableLong      object_count;
		DB::BindableNetworkId object_id;

	  private:
		std::string m_schema;
	};
};

// ======================================================================

#endif
