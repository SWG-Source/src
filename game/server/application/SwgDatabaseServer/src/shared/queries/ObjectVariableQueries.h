// ======================================================================
//
// ObjectVariableQueries.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ObjectVariableQueries_H
#define INCLUDED_ObjectVariableQueries_H

// ======================================================================

#include "SwgDatabaseServer/Schema.h"
#include "serverDatabase/DatabaseProcessQuery.h"
#include "sharedDatabaseInterface/DbBindableVarray.h"
#include "sharedDatabaseInterface/DbModeQuery.h"
#include "sharedFoundation/NetworkId.h"

#include <vector>

// ======================================================================

namespace DBQuery
{
	/**
	 * Base class for queries to add or update object variables
	 */
	class GenericObjectVariableQuery : public DatabaseProcessQuery
	{
		GenericObjectVariableQuery(const GenericObjectVariableQuery&); // disable
		GenericObjectVariableQuery& operator=(const GenericObjectVariableQuery&); // disable
	  public:
		GenericObjectVariableQuery();
		
		virtual bool bindParameters();
		virtual bool bindColumns();

		bool setupData(DB::Session *session);
		bool addData(const NetworkId &objectId, int nameId, int typeId, const std::string &value);
		void clearData();
		void freeData();

		int getNumItems() const;
		
	  private:
		DB::BindableVarrayString m_objectIds;
		DB::BindableVarrayNumber m_nameIds;
		DB::BindableVarrayNumber m_types;
		DB::BindableVarrayString m_values;
		DB::BindableLong         m_numItems;
	};

	class AddObjectVariableQuery : public GenericObjectVariableQuery
	{
	  public:
		AddObjectVariableQuery();
		virtual void getSQL(std::string &sql);

	  private:
		AddObjectVariableQuery(const AddObjectVariableQuery&);
		AddObjectVariableQuery& operator=(const AddObjectVariableQuery&);
	};

	class UpdateObjectVariableQuery : public GenericObjectVariableQuery
	{
	  public:
		UpdateObjectVariableQuery();
		virtual void getSQL(std::string &sql);

	  private:
		UpdateObjectVariableQuery(const UpdateObjectVariableQuery&);
		UpdateObjectVariableQuery& operator=(const UpdateObjectVariableQuery&);
	};
	
	class RemoveObjectVariableQuery : public DatabaseProcessQuery
	{
		RemoveObjectVariableQuery(const RemoveObjectVariableQuery&); // disable
		RemoveObjectVariableQuery& operator=(const RemoveObjectVariableQuery&); // disable
	  public:
		RemoveObjectVariableQuery();
		
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual void getSQL(std::string &sql);

		bool setupData(DB::Session *session);
		bool addData(const NetworkId &objectId, int nameId);
		void clearData();
		void freeData();

		int getNumItems() const;
				
	  private:
		DB::BindableVarrayString m_objectIds;
		DB::BindableVarrayNumber m_nameIds;
		DB::BindableLong         m_numItems;
	};

	/** 
	 * Query to load all the object variables for an object at the same
	 * time.
	 */
	class GetAllObjectVariables : public DB::Query
	{
	  public:
		GetAllObjectVariables(const std::string &schema);
		
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual void getSQL(std::string &sql);

		const std::vector<DBSchema::ObjectVariableRow> &getData() const;

		QueryMode getExecutionMode() const;

	  protected:
		std::vector<DBSchema::ObjectVariableRow> m_data;
		const std::string m_schema;
		
	  private:
		GetAllObjectVariables(const GetAllObjectVariables&); // disable
		GetAllObjectVariables& operator=(const GetAllObjectVariables&); // disable
	};

	/**
	 * Get all the object variables from the local database that override
	 * those in the gold database.
	 */
	class GetObjectVariableOverrides : public GetAllObjectVariables
	{
	  public:
		GetObjectVariableOverrides(const std::string &schema, const std::string &goldSchema);

		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();

	  private:
		DB::BindableString<50> gold_schema;

	  private:
		GetObjectVariableOverrides(const GetObjectVariableOverrides&); // disable
		GetObjectVariableOverrides& operator=(const GetObjectVariableOverrides&); // disable		
	};
} 

// ======================================================================

#endif
