// ======================================================================
//
// ContentsLocator.h
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ContentsLocator_H
#define INCLUDED_ContentsLocator_H

#include "serverDatabase/ObjectLocator.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedFoundation/NetworkId.h"

// ======================================================================

/**
 * Locate a specified character and add it to the list of objects to be
 * loaded by the Snapshot.
 */
class ContentsLocator : public ObjectLocator
{
  public:
	ContentsLocator            (const NetworkId &containerId);
	virtual ~ContentsLocator   ();
	
  public:
	virtual bool locateObjects (DB::Session *session, const std::string &schema, int &objectsLocated);
	virtual void sendPostBaselinesCustomData (GameServerConnection &conn) const;
	
  private:
	NetworkId m_containerId;

  private:
	class LocateObjectQuery : public DB::Query
	{
	  public:
		LocateObjectQuery                (const NetworkId &containerId, const std::string &schema);

		virtual void getSQL              (std::string &sql);
		virtual bool bindParameters      ();
		virtual bool bindColumns         ();

	  private:
		virtual DB::Query::QueryMode getExecutionMode() const;
		
	  private:
		LocateObjectQuery                (const LocateObjectQuery&); // disable
		LocateObjectQuery& operator=     (const LocateObjectQuery&); // disable

	  public:
		DB::BindableNetworkId container_id;

	  private:
		std::string m_schema;
	};
};

// ======================================================================

#endif
