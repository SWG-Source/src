// ======================================================================
//
// CharacterQueries.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/CharacterQueries.h"

#include "serverDatabase/DatabaseProcess.h"

using namespace DBQuery;

// ======================================================================

void GetCharacters::getSQL(std::string &sql)
{
	sql="begin :result := "+DatabaseProcess::getInstance().getSchemaQualifier()+"loader.get_characters(:station_id); end;";
}

// ----------------------------------------------------------------------

bool GetCharacters::bindParameters()
{
	if (!bindParameter(data.station_id)) return false;

	return true;
}

// ----------------------------------------------------------------------

bool GetCharacters::bindColumns()
{
	if (!bindCol(data.object_id)) return false;
	if (!bindCol(data.object_template)) return false;
	if (!bindCol(data.scene_id)) return false;
	if (!bindCol(data.character_name)) return false;
	if (!bindCol(data.container_id)) return false;
	if (!bindCol(data.x)) return false;
	if (!bindCol(data.y)) return false;
	if (!bindCol(data.z)) return false;
	
	return true;
}

// ----------------------------------------------------------------------

const GetCharacters::GetCharactersRow &GetCharacters::getData() const
{
	return data;
}

// ----------------------------------------------------------------------

void GetCharacters::setStationId(StationId station_id)
{
	data.station_id.setValue(static_cast<int32>(station_id));
}
		
// ----------------------------------------------------------------------

DB::Query::QueryMode GetCharacters::getExecutionMode() const
{
	return MODE_PLSQL_REFCURSOR;
}

// ======================================================================

AddCharacter::AddCharacter(StationId stationId, const NetworkId &objectId, const Unicode::String &characterName, const std::string &normalizedName) :
		DB::Query(),
		station_id(static_cast<int32>(stationId)),
		object_id(objectId),
		character_name(characterName),
		normalized_name(normalizedName)
{
}

// ----------------------------------------------------------------------

void AddCharacter::getSQL(std::string &sql)
{
	sql="begin "+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.add_player (:station_id, :character_object, :character_name, :normalized_name); end;";
// insert into players (station_id, character_object) values (?,?)");
}

// ----------------------------------------------------------------------

bool AddCharacter::bindParameters()
{
	if (!bindParameter(station_id)) return false;
	if (!bindParameter(object_id)) return false;
	if (!bindParameter(character_name)) return false;
	if (!bindParameter(normalized_name)) return false;

	return true;
}

// ----------------------------------------------------------------------

bool AddCharacter::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode AddCharacter::getExecutionMode() const
{
	return MODE_PROCEXEC;
}

// ======================================================================

RenameCharacter::RenameCharacter(const NetworkId &characterId, const Unicode::String &characterName, const Unicode::String &normalizedName) :
		DB::Query(),
		object_id(characterId),
		character_name(characterName),
		normalized_name(normalizedName),
		result()
{
}

// ----------------------------------------------------------------------

void RenameCharacter::getSQL(std::string &sql)
{
	sql=std::string("begin :result := ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.rename_character (:character_object, :character_name, :normalized_name); end;";
}

// ----------------------------------------------------------------------

bool RenameCharacter::bindParameters()
{
	if (!bindParameter(result)) return false;
	if (!bindParameter(object_id)) return false;
	if (!bindParameter(character_name)) return false;
	if (!bindParameter(normalized_name)) return false;

	return true;
}

// ----------------------------------------------------------------------

bool RenameCharacter::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

DB::Query::QueryMode RenameCharacter::getExecutionMode() const
{
	return MODE_PROCEXEC;
}

// ----------------------------------------------------------------------

bool RenameCharacter::getResult() const
{
	return (result.getValue()==1);
}

// ======================================================================
