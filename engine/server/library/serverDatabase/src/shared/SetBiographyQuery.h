// ======================================================================
//
// SetBiographyQuery.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SetBiographyQuery_H
#define INCLUDED_SetBiographyQuery_H

// ======================================================================

#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"

// ======================================================================

namespace DBQuery {

	class SetBiographyQuery : public DB::Query
	{
	public:
	
		SetBiographyQuery(const NetworkId &newOwner, const Unicode::String &newBio);
		
		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();

	  protected:
		virtual QueryMode getExecutionMode() const;

	  private:
		DB::BindableNetworkId owner;
		DB::BindableUnicode<1024> bio;

	  private:
		SetBiographyQuery(const SetBiographyQuery&); // disable
		SetBiographyQuery& operator=(const SetBiographyQuery&); //disable
	};

} //namespace

// ======================================================================

#endif
