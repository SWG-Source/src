// ======================================================================
//
// WorldContainerLocator.h
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_WorldContainerLocator_H
#define INCLUDED_WorldContainerLocator_H

#include "serverDatabase/ObjectLocator.h"
#include "sharedDatabaseInterface/DbBindableVarray.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedFoundation/NetworkId.h"

// ======================================================================

/**
 * Locate all objects whose load_with matches objects in the list.
 * This is intended to locate objects in the live database who are persisted
 * inside objects in the gold database.
 */
class WorldContainerLocator : public ObjectLocator
{
  public:
	WorldContainerLocator(const std::vector<NetworkId> &containers);
	virtual ~WorldContainerLocator();
	
  public:
	virtual bool locateObjects(DB::Session *session, const std::string &schema, int &objectsLocated);

  private:
	std::vector<NetworkId> * m_containers;

  private:
	WorldContainerLocator &operator=(const WorldContainerLocator&); //disable
	WorldContainerLocator(const WorldContainerLocator&); //disable

  private:
	class WorldContainerQuery : public DB::Query
	{
	  public:
		WorldContainerQuery(const std::string &schema);

		virtual void getSQL              (std::string &sql);
		virtual bool bindParameters      ();
		virtual bool bindColumns         ();

		bool setupData(DB::Session *session);
		void clearData();
		void freeData();
		bool addContainer(const NetworkId &containerId);		
		int  getNumItems() const;
		int  getObjectCount() const;

	  private:
		virtual DB::Query::QueryMode getExecutionMode() const;
		
	  private:
		DB::BindableVarrayString	 container_ids;
		DB::BindableLong             num_containers;
		DB::BindableLong             num_objects_found;
		std::string                  m_schema;

	  private:
		WorldContainerQuery             (const WorldContainerQuery&); // disable
		WorldContainerQuery& operator=  (const WorldContainerQuery&); //disable
	};
};

// ======================================================================

#endif
