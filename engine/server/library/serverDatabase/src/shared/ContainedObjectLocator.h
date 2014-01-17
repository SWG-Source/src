// ======================================================================
//
// ContainedObjectLocator.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ContainedObjectLocator_H
#define INCLUDED_ContainedObjectLocator_H

#include "serverDatabase/ObjectLocator.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedFoundation/NetworkId.h"

// ======================================================================

/**
 * Locate a specified character and add it to the list of objects to be
 * loaded by the Snapshot.
 */
class ContainedObjectLocator : public ObjectLocator
{
  public:
	ContainedObjectLocator           (const NetworkId &containerId, const NetworkId &objectId);
	virtual ~ContainedObjectLocator  ();
	
  public:
	virtual bool locateObjects       (DB::Session *session, const std::string &schema, int &objectsLocated);
	virtual void sendPostBaselinesCustomData (GameServerConnection &conn) const;
	
  private:
	NetworkId m_containerId;
	NetworkId m_objectId;

  private:
	class LocateObjectQuery : public DB::Query
	{
	  public:
		LocateObjectQuery                (const NetworkId &containerId, const NetworkId &objectId, const std::string &schema);

		virtual void getSQL              (std::string &sql);
		virtual bool bindParameters      ();
		virtual bool bindColumns         ();

	  private:
		virtual DB::Query::QueryMode getExecutionMode() const;
		
	  private:
		LocateObjectQuery             (const LocateObjectQuery&); // disable
		LocateObjectQuery& operator=  (const LocateObjectQuery&); //disable

	  public:
		DB::BindableNetworkId container_id;
		DB::BindableNetworkId object_id;

	  private:
		std::string m_schema;
	};
};

// ======================================================================

#endif
