// ======================================================================
//
// CharacterQueries.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_StructureQueries_H
#define INCLUDED_StructureQueries_H

// ======================================================================

#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbRow.h"
#include "sharedFoundation/StationId.h"

// ======================================================================

namespace DBQuery {

	/** A query to get the list of characters for an account.
	 */
	class GetStructures : public DB::Query
	{
	public:
		
		struct GetStructuresRow : public DB::Row
		{
			DB::BindableString<128> scene_id;
			DB::BindableNetworkId  object_id;
			DB::BindableString<1000> object_template;
			DB::BindableDouble x;
			DB::BindableDouble y;
			DB::BindableDouble z;
			DB::BindableNetworkId character_id;
			DB::BindableLong deleted;
		};
		
		GetStructures() {}
		
		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();

		const GetStructuresRow &getData() const;
		void setCharacterId(NetworkId & characterId);

	  protected:
		virtual QueryMode getExecutionMode() const;

	  private:
		GetStructuresRow data;

	  private:
		GetStructures(const GetStructures&); // disable
		GetStructures& operator=(const GetStructures&); //disable
	};

}

// ======================================================================

#endif
