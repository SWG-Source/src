// ======================================================================
//
// GetBiographyQuery.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_GetBiographyQuery_H
#define INCLUDED_GetBiographyQuery_H

// ======================================================================

#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"

// ======================================================================

namespace DBQuery {

	class GetBiographyQuery : public DB::Query
	{
	public:
	
		GetBiographyQuery(const NetworkId &newOwner);
		
		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();

		void getBio(Unicode::String &buffer) const;

	  protected:
		virtual QueryMode getExecutionMode() const;

	  private:
		DB::BindableNetworkId owner;
		DB::BindableUnicode<1024> bio;

	  private:
		GetBiographyQuery(const GetBiographyQuery&); // disable
		GetBiographyQuery& operator=(const GetBiographyQuery&); //disable
	};

} //namespace

// ======================================================================

#endif
