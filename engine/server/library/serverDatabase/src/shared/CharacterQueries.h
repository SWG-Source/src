// ======================================================================
//
// CharacterQueries.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CharacterQueries_H
#define INCLUDED_CharacterQueries_H

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
	class GetCharacters : public DB::Query
	{
	public:
		
		struct GetCharactersRow : public DB::Row
		{
			DB::BindableString<128> character_name;
			DB::BindableString<128> scene_id;
			DB::BindableNetworkId  object_id;
			DB::BindableString<1000> object_template;
			DB::BindableNetworkId  container_id;
			DB::BindableDouble x;
			DB::BindableDouble y;
			DB::BindableDouble z;
			DB::BindableLong station_id;
		};
		
		GetCharacters() {}
		
		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();

		const GetCharactersRow &getData() const;
		void setStationId(StationId station_id);

	  protected:
		virtual QueryMode getExecutionMode() const;

	  private:
		GetCharactersRow data;

	  private:
		GetCharacters(const GetCharacters&); // disable
		GetCharacters& operator=(const GetCharacters&); //disable
	};

// ======================================================================

    /** A query to associate a character with a particular account
	 */
	class AddCharacter : public DB::Query
	{
	public:
		AddCharacter(StationId stationId, const NetworkId &objectId, const Unicode::String &characterName, const std::string &normalizedName);
		
		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();

	  protected:
		virtual QueryMode getExecutionMode() const;

	  private:
		DB::BindableLong      station_id;
		DB::BindableNetworkId object_id;
		DB::BindableString<127> character_name;
		DB::BindableString<127> normalized_name;
		
	  private:
		AddCharacter(const AddCharacter&); // disable
		AddCharacter& operator=(const AddCharacter&); //disable
	};

// ======================================================================

	class RenameCharacter : public DB::Query
	{
	  public:
		RenameCharacter                     (const NetworkId &characterId, const Unicode::String &characterName, const Unicode::String &normalizedName);
		bool getResult                      () const;
	
		virtual void getSQL                 (std::string &sql);
		virtual bool bindParameters         ();
		virtual bool bindColumns            ();

	  protected:
		virtual QueryMode getExecutionMode  () const;

	  private:
		DB::BindableNetworkId    object_id;
		DB::BindableString<127>  character_name;
		DB::BindableString<127>  normalized_name;
		DB::BindableLong         result;
		
	  private:
		RenameCharacter                     (const RenameCharacter&); // disable
		RenameCharacter& operator=          (const RenameCharacter&); //disable
	};
}

// ======================================================================

#endif
