// ======================================================================
//
// LfgCharacterData.cpp
// Copyright 2008 Sony Online Entertainment LLC (SOE)
// All rights reserved.
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/LfgCharacterData.h"

#include "sharedFoundation/CalendarTime.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedGame/LfgDataTable.h"
#include "sharedGame/PvpData.h"
#include "unicodeArchive/UnicodeArchive.h"
#include "UnicodeUtils.h"

#include <algorithm>

// ======================================================================

namespace LfgCharacterDataNamespace
{
	std::vector<Unicode::String> s_professionDisplayString;

	// ***strings used to display connected character statistics in SOEMon***

	// searchable/anonymous
	std::string const s_stat_searchable_section("searchable");
	std::string const s_stat_anonymous(s_stat_searchable_section + ".anonymous");
	std::string const s_stat_searchable(s_stat_searchable_section + ".searchable");

	// active/inactive
	std::string const s_stat_active_section("active");
	std::string const s_stat_active(s_stat_active_section + ".active");
	std::string const s_stat_inactive(s_stat_active_section + ".inactive");

	// grouped/ungrouped
	std::string const s_stat_grouped_section("grouped");
	std::string const s_stat_grouped(s_stat_grouped_section + ".grouped");
	std::string const s_stat_ungrouped(s_stat_grouped_section + ".ungrouped");

	// guilded/unguilded
	std::string const s_stat_guilded_section("guilded");
	std::string const s_stat_guilded(s_stat_guilded_section + ".guilded");
	std::string const s_stat_unguilded(s_stat_guilded_section + ".unguilded");

	// searchableByCtsSourceGalaxy
	std::string const s_stat_searchablebyctssourcegalaxy_section("searchableByCtsSourceGalaxy");
	std::string const s_stat_searchablebyctssourcegalaxy_yes(s_stat_searchablebyctssourcegalaxy_section + ".yes");
	std::string const s_stat_searchablebyctssourcegalaxy_no(s_stat_searchablebyctssourcegalaxy_section + ".no");

	// displayLocationInSearchResults
	std::string const s_stat_displayLocation_section("displayLocation");
	std::string const s_stat_displayLocation(s_stat_displayLocation_section + ".displayLocation");
	std::string const s_stat_hideLocation(s_stat_displayLocation_section + ".hideLocation");

	// gender
	std::string const s_stat_gender_section("gender");
	std::string const s_stat_female(s_stat_gender_section + ".female");
	std::string const s_stat_male(s_stat_gender_section + ".male");

	// faction
	std::string const s_stat_faction_section("faction");
	std::string const s_stat_faction_imperial(s_stat_faction_section + ".imperial");
	std::string const s_stat_faction_neutral(s_stat_faction_section + ".neutral");
	std::string const s_stat_faction_rebel(s_stat_faction_section + ".rebel");

	// pilot
	std::string const s_stat_pilot_section("pilot");
	std::string const s_stat_pilot_imperial(s_stat_pilot_section + ".imperial");
	std::string const s_stat_pilot_neutral(s_stat_pilot_section + ".neutral");
	std::string const s_stat_pilot_none(s_stat_pilot_section + ".none");
	std::string const s_stat_pilot_rebel(s_stat_pilot_section + ".rebel");

	// species
	std::string const s_stat_species_section("species");
	std::string const s_stat_bothan(s_stat_species_section + ".bothan");
	std::string const s_stat_human(s_stat_species_section + ".human");
	std::string const s_stat_ithorian(s_stat_species_section + ".ithorian");
	std::string const s_stat_monCalamari(s_stat_species_section + ".monCalamari");
	std::string const s_stat_rodian(s_stat_species_section + ".rodian");
	std::string const s_stat_sullustan(s_stat_species_section + ".sullustan");
	std::string const s_stat_trandoshan(s_stat_species_section + ".trandoshan");
	std::string const s_stat_twilek(s_stat_species_section + ".twilek");
	std::string const s_stat_wookiee(s_stat_species_section + ".wookiee");
	std::string const s_stat_zabrak(s_stat_species_section + ".zabrak");

	// species by gender
	std::string const s_stat_species_gender_section("speciesGender");
	std::string const s_stat_bothan_m(s_stat_species_gender_section + ".bothan_m");
	std::string const s_stat_bothan_f(s_stat_species_gender_section + ".bothan_f");
	std::string const s_stat_human_m(s_stat_species_gender_section + ".human_m");
	std::string const s_stat_human_f(s_stat_species_gender_section + ".human_f");
	std::string const s_stat_ithorian_m(s_stat_species_gender_section + ".ithorian_m");
	std::string const s_stat_ithorian_f(s_stat_species_gender_section + ".ithorian_f");
	std::string const s_stat_monCalamari_m(s_stat_species_gender_section + ".monCalamari_m");
	std::string const s_stat_monCalamari_f(s_stat_species_gender_section + ".monCalamari_f");
	std::string const s_stat_rodian_m(s_stat_species_gender_section + ".rodian_m");
	std::string const s_stat_rodian_f(s_stat_species_gender_section + ".rodian_f");
	std::string const s_stat_sullustan_m(s_stat_species_gender_section + ".sullustan_m");
	std::string const s_stat_sullustan_f(s_stat_species_gender_section + ".sullustan_f");
	std::string const s_stat_trandoshan_m(s_stat_species_gender_section + ".trandoshan_m");
	std::string const s_stat_trandoshan_f(s_stat_species_gender_section + ".trandoshan_f");
	std::string const s_stat_twilek_m(s_stat_species_gender_section + ".twilek_m");
	std::string const s_stat_twilek_f(s_stat_species_gender_section + ".twilek_f");
	std::string const s_stat_wookiee_m(s_stat_species_gender_section + ".wookiee_m");
	std::string const s_stat_wookiee_f(s_stat_species_gender_section + ".wookiee_f");
	std::string const s_stat_zabrak_m(s_stat_species_gender_section + ".zabrak_m");
	std::string const s_stat_zabrak_f(s_stat_species_gender_section + ".zabrak_f");

	// profession
	std::string const s_stat_profession_section("profession");
	std::string const s_stat_bountyHunter(s_stat_profession_section + ".bountyHunter");
	std::string const s_stat_commando(s_stat_profession_section + ".commando");
	std::string const s_stat_entertainer(s_stat_profession_section + ".entertainer");
	std::string const s_stat_jedi(s_stat_profession_section + ".jedi");
	std::string const s_stat_medic(s_stat_profession_section + ".medic");
	std::string const s_stat_officer(s_stat_profession_section + ".officer");
	std::string const s_stat_smuggler(s_stat_profession_section + ".smuggler");
	std::string const s_stat_spy(s_stat_profession_section + ".spy");
	std::string const s_stat_traderDomestics(s_stat_profession_section + ".traderDomestics");
	std::string const s_stat_traderEngineering(s_stat_profession_section + ".traderEngineering");
	std::string const s_stat_traderMunitions(s_stat_profession_section + ".traderMunitions");
	std::string const s_stat_traderStructures(s_stat_profession_section + ".traderStructures");

	// profession by gender
	std::string const s_stat_profession_gender_section("professionGender");
	std::string const s_stat_bountyHunter_m(s_stat_profession_gender_section + ".bountyHunter_m");
	std::string const s_stat_bountyHunter_f(s_stat_profession_gender_section + ".bountyHunter_f");
	std::string const s_stat_commando_m(s_stat_profession_gender_section + ".commando_m");
	std::string const s_stat_commando_f(s_stat_profession_gender_section + ".commando_f");
	std::string const s_stat_entertainer_m(s_stat_profession_gender_section + ".entertainer_m");
	std::string const s_stat_entertainer_f(s_stat_profession_gender_section + ".entertainer_f");
	std::string const s_stat_jedi_m(s_stat_profession_gender_section + ".jedi_m");
	std::string const s_stat_jedi_f(s_stat_profession_gender_section + ".jedi_f");
	std::string const s_stat_medic_m(s_stat_profession_gender_section + ".medic_m");
	std::string const s_stat_medic_f(s_stat_profession_gender_section + ".medic_f");
	std::string const s_stat_officer_m(s_stat_profession_gender_section + ".officer_m");
	std::string const s_stat_officer_f(s_stat_profession_gender_section + ".officer_f");
	std::string const s_stat_smuggler_m(s_stat_profession_gender_section + ".smuggler_m");
	std::string const s_stat_smuggler_f(s_stat_profession_gender_section + ".smuggler_f");
	std::string const s_stat_spy_m(s_stat_profession_gender_section + ".spy_m");
	std::string const s_stat_spy_f(s_stat_profession_gender_section + ".spy_f");
	std::string const s_stat_traderDomestics_m(s_stat_profession_gender_section + ".traderDomestics_m");
	std::string const s_stat_traderDomestics_f(s_stat_profession_gender_section + ".traderDomestics_f");
	std::string const s_stat_traderEngineering_m(s_stat_profession_gender_section + ".traderEngineering_m");
	std::string const s_stat_traderEngineering_f(s_stat_profession_gender_section + ".traderEngineering_f");
	std::string const s_stat_traderMunitions_m(s_stat_profession_gender_section + ".traderMunitions_m");
	std::string const s_stat_traderMunitions_f(s_stat_profession_gender_section + ".traderMunitions_f");
	std::string const s_stat_traderStructures_m(s_stat_profession_gender_section + ".traderStructures_m");
	std::string const s_stat_traderStructures_f(s_stat_profession_gender_section + ".traderStructures_f");

	// level
	std::string const s_stat_level_section("level");
	std::string const s_stat_level_10(s_stat_level_section + ".01_10");
	std::string const s_stat_level_10_imperial(s_stat_level_10 + ".imperial");
	std::string const s_stat_level_10_neutral(s_stat_level_10 + ".neutral");
	std::string const s_stat_level_10_rebel(s_stat_level_10 + ".rebel");
	std::string const s_stat_level_20(s_stat_level_section + ".11_20");
	std::string const s_stat_level_20_imperial(s_stat_level_20 + ".imperial");
	std::string const s_stat_level_20_neutral(s_stat_level_20 + ".neutral");
	std::string const s_stat_level_20_rebel(s_stat_level_20 + ".rebel");
	std::string const s_stat_level_30(s_stat_level_section + ".21_30");
	std::string const s_stat_level_30_imperial(s_stat_level_30 + ".imperial");
	std::string const s_stat_level_30_neutral(s_stat_level_30 + ".neutral");
	std::string const s_stat_level_30_rebel(s_stat_level_30 + ".rebel");
	std::string const s_stat_level_40(s_stat_level_section + ".31_40");
	std::string const s_stat_level_40_imperial(s_stat_level_40 + ".imperial");
	std::string const s_stat_level_40_neutral(s_stat_level_40 + ".neutral");
	std::string const s_stat_level_40_rebel(s_stat_level_40 + ".rebel");
	std::string const s_stat_level_50(s_stat_level_section + ".41_50");
	std::string const s_stat_level_50_imperial(s_stat_level_50 + ".imperial");
	std::string const s_stat_level_50_neutral(s_stat_level_50 + ".neutral");
	std::string const s_stat_level_50_rebel(s_stat_level_50 + ".rebel");
	std::string const s_stat_level_60(s_stat_level_section + ".51_60");
	std::string const s_stat_level_60_imperial(s_stat_level_60 + ".imperial");
	std::string const s_stat_level_60_neutral(s_stat_level_60 + ".neutral");
	std::string const s_stat_level_60_rebel(s_stat_level_60 + ".rebel");
	std::string const s_stat_level_70(s_stat_level_section + ".61_70");
	std::string const s_stat_level_70_imperial(s_stat_level_70 + ".imperial");
	std::string const s_stat_level_70_neutral(s_stat_level_70 + ".neutral");
	std::string const s_stat_level_70_rebel(s_stat_level_70 + ".rebel");
	std::string const s_stat_level_80(s_stat_level_section + ".71_80");
	std::string const s_stat_level_80_imperial(s_stat_level_80 + ".imperial");
	std::string const s_stat_level_80_neutral(s_stat_level_80 + ".neutral");
	std::string const s_stat_level_80_rebel(s_stat_level_80 + ".rebel");
	std::string const s_stat_level_89(s_stat_level_section + ".81_89");
	std::string const s_stat_level_89_imperial(s_stat_level_89 + ".imperial");
	std::string const s_stat_level_89_neutral(s_stat_level_89 + ".neutral");
	std::string const s_stat_level_89_rebel(s_stat_level_89 + ".rebel");
	std::string const s_stat_level_90(s_stat_level_section + ".90");
	std::string const s_stat_level_90_imperial(s_stat_level_90 + ".imperial");
	std::string const s_stat_level_90_neutral(s_stat_level_90 + ".neutral");
	std::string const s_stat_level_90_rebel(s_stat_level_90 + ".rebel");

	// location planet
	std::string const s_stat_location_section("location");

	// ***MUST*** contain all the planets that are started
	// ***MUST*** be named exactly like the planet name in the startPlanet= line
	std::string const s_all_planets[] = {
		"tutorial",
		"tatooine",
		"naboo",
		"corellia",
		"rori",
		"talus",
		"yavin4",
		"endor",
		"lok",
		"dantooine",
		"dathomir",
		"dungeon1",
		"adventure1",
		"adventure2",
		"space_corellia",
		"space_naboo",
		"space_tatooine",
		"space_lok",
		"space_dantooine",
		"space_dathomir",
		"space_yavin4",
		"space_endor",
		"space_heavy1",
		"space_light1",
		"kashyyyk_main",
		"kashyyyk_pob_dungeons",
		"kashyyyk_south_dungeons",
		"kashyyyk_north_dungeons",
		"kashyyyk_rryatt_trail",
		"kashyyyk_hunting",
		"kashyyyk_dead_forest",
		"space_kashyyyk",
		"space_npe_falcon",
		"space_npe_falcon_2",
		"space_npe_falcon_3",
		"space_ord_mantell",
		"mustafar",
		"space_nova_orion",
		"" // last one must be blank/empty
	};

	// activities
	std::string const s_stat_activities_section("activities");
	std::map<uint32, std::string> s_activities;
	void initializeActivitiesList()
	{
		// these are also defined in session.scriptlib so ***MUST*** be kept in sync
		s_activities[1]          = s_stat_activities_section + ".pve";
		s_activities[2]          = s_stat_activities_section + ".pvp";
		s_activities[4]          = s_stat_activities_section + ".crafting";
		s_activities[8]          = s_stat_activities_section + ".entertainPerformed";
		s_activities[16]         = s_stat_activities_section + ".entertainObserved";
		s_activities[32]         = s_stat_activities_section + ".terminalBazaar";
		s_activities[64]         = s_stat_activities_section + ".terminalMission";
		s_activities[128]        = s_stat_activities_section + ".spaceLaunch";
		s_activities[256]        = s_stat_activities_section + ".decorate";
		s_activities[512]        = s_stat_activities_section + ".accessVendorOwner";
		s_activities[1024]       = s_stat_activities_section + ".accessVendorOther";
		s_activities[2048]       = s_stat_activities_section + ".accessHarvester";
		s_activities[4096]       = s_stat_activities_section + ".accessFactory";
		s_activities[8192]       = s_stat_activities_section + ".accessStructure";
		s_activities[16384]      = s_stat_activities_section + ".fishing";
	}

	// character create time
	std::string const s_stat_character_create_time_section("characterCreateTime");
	std::string const s_stat_character_create_time_001day(s_stat_character_create_time_section + ".createdWithinPast_Day001");
	std::string const s_stat_character_create_time_002day(s_stat_character_create_time_section + ".createdWithinPast_Day002");
	std::string const s_stat_character_create_time_003day(s_stat_character_create_time_section + ".createdWithinPast_Day003");
	std::string const s_stat_character_create_time_004day(s_stat_character_create_time_section + ".createdWithinPast_Day004");
	std::string const s_stat_character_create_time_005day(s_stat_character_create_time_section + ".createdWithinPast_Day005");
	std::string const s_stat_character_create_time_006day(s_stat_character_create_time_section + ".createdWithinPast_Day006");
	std::string const s_stat_character_create_time_007day(s_stat_character_create_time_section + ".createdWithinPast_Day007");
	std::string const s_stat_character_create_time_014day(s_stat_character_create_time_section + ".createdWithinPast_Day014");
	std::string const s_stat_character_create_time_021day(s_stat_character_create_time_section + ".createdWithinPast_Day021");
	std::string const s_stat_character_create_time_030day(s_stat_character_create_time_section + ".createdWithinPast_Day030");
	std::string const s_stat_character_create_time_060day(s_stat_character_create_time_section + ".createdWithinPast_Day060");
	std::string const s_stat_character_create_time_090day(s_stat_character_create_time_section + ".createdWithinPast_Day090");
	std::string const s_stat_character_create_time_180day(s_stat_character_create_time_section + ".createdWithinPast_Day180");
	std::string const s_stat_character_create_time_270day(s_stat_character_create_time_section + ".createdWithinPast_Day270");
	std::string const s_stat_character_create_time_1year (s_stat_character_create_time_section + ".createdWithinPast_Year1");
	std::string const s_stat_character_create_time_2year (s_stat_character_create_time_section + ".createdWithinPast_Year2");
	std::string const s_stat_character_create_time_3year (s_stat_character_create_time_section + ".createdWithinPast_Year3");

	// character age
	std::string const s_stat_character_age_section("characterAge");
	std::string const s_stat_character_age_001day(s_stat_character_age_section + ".lessThanOrEqual_Day001");
	std::string const s_stat_character_age_002day(s_stat_character_age_section + ".lessThanOrEqual_Day002");
	std::string const s_stat_character_age_003day(s_stat_character_age_section + ".lessThanOrEqual_Day003");
	std::string const s_stat_character_age_004day(s_stat_character_age_section + ".lessThanOrEqual_Day004");
	std::string const s_stat_character_age_005day(s_stat_character_age_section + ".lessThanOrEqual_Day005");
	std::string const s_stat_character_age_006day(s_stat_character_age_section + ".lessThanOrEqual_Day006");
	std::string const s_stat_character_age_007day(s_stat_character_age_section + ".lessThanOrEqual_Day007");
	std::string const s_stat_character_age_014day(s_stat_character_age_section + ".lessThanOrEqual_Day014");
	std::string const s_stat_character_age_021day(s_stat_character_age_section + ".lessThanOrEqual_Day021");
	std::string const s_stat_character_age_030day(s_stat_character_age_section + ".lessThanOrEqual_Day030");
	std::string const s_stat_character_age_060day(s_stat_character_age_section + ".lessThanOrEqual_Day060");
	std::string const s_stat_character_age_090day(s_stat_character_age_section + ".lessThanOrEqual_Day090");
	std::string const s_stat_character_age_180day(s_stat_character_age_section + ".lessThanOrEqual_Day180");
	std::string const s_stat_character_age_270day(s_stat_character_age_section + ".lessThanOrEqual_Day270");
	std::string const s_stat_character_age_1year (s_stat_character_age_section + ".lessThanOrEqual_Year1");
	std::string const s_stat_character_age_2year (s_stat_character_age_section + ".lessThanOrEqual_Year2");
	std::string const s_stat_character_age_3year (s_stat_character_age_section + ".lessThanOrEqual_Year3");

	// ***strings used to display connected character statistics in SOEMon***
}

using namespace LfgCharacterDataNamespace;

// ======================================================================

namespace Archive
{
	void get(ReadIterator & source, LfgCharacterData & target)
	{
		get(source, target.characterId);
		get(source, target.characterCreateTime);
		get(source, target.characterAge);
		get(source, target.characterName);
		get(source, target.characterLowercaseFirstName);

		uint16 uShort = 0;
		get(source, uShort);
		target.species = static_cast<SharedCreatureObjectTemplate::Species>(uShort);

		uint8 uChar = 0;
		get(source, uChar);
		target.gender = static_cast<SharedCreatureObjectTemplate::Gender>(uChar);

		get(source, uChar);
		target.profession = static_cast<LfgCharacterData::Profession>(uChar);

		get(source, uChar);
		target.pilot = static_cast<LfgCharacterData::Pilot>(uChar);

		get(source, target.level);
		get(source, target.faction);
		get(source, target.guildName);
		get(source, target.guildAbbrev);
		get(source, target.citizenOfCity);
		get(source, target.groupId);
		get(source, target.locationPlanet);
		get(source, target.locationRegion);
		get(source, target.locationFactionalPresenceGcwRegion);
		get(source, target.locationFactionalPresenceGridX);
		get(source, target.locationFactionalPresenceGridZ);
		get(source, target.locationPlayerCity);
		get(source, target.ctsSourceGalaxy);

		get(source, uChar);
		target.searchableByCtsSourceGalaxy = ((uChar == 0) ? false : true);

		get(source, uChar);
		target.displayLocationInSearchResults = ((uChar == 0) ? false : true);

		get(source, uChar);
		target.anonymous = ((uChar == 0) ? false : true);

		get(source, uChar);
		target.active = ((uChar == 0) ? false : true);

		get(source, target.characterInterests);
		get(source, target.sessionActivity);
	}

	void put(ByteStream & target, const LfgCharacterData & source)
	{
		put(target, source.characterId);
		put(target, source.characterCreateTime);
		put(target, source.characterAge);
		put(target, source.characterName);
		put(target, source.characterLowercaseFirstName);
		put(target, static_cast<uint16>(source.species));
		put(target, static_cast<uint8>(source.gender));
		put(target, static_cast<uint8>(source.profession));
		put(target, static_cast<uint8>(source.pilot));
		put(target, source.level);
		put(target, source.faction);
		put(target, source.guildName);
		put(target, source.guildAbbrev);
		put(target, source.citizenOfCity);
		put(target, source.groupId);
		put(target, source.locationPlanet);
		put(target, source.locationRegion);
		put(target, source.locationFactionalPresenceGcwRegion);
		put(target, source.locationFactionalPresenceGridX);
		put(target, source.locationFactionalPresenceGridZ);
		put(target, source.locationPlayerCity);
		put(target, source.ctsSourceGalaxy);
		put(target, static_cast<uint8>(source.searchableByCtsSourceGalaxy ? 1 : 0));
		put(target, static_cast<uint8>(source.displayLocationInSearchResults ? 1 : 0));
		put(target, static_cast<uint8>(source.anonymous ? 1 : 0));
		put(target, static_cast<uint8>(source.active ? 1 : 0));
		put(target, source.characterInterests);
		put(target, source.sessionActivity);
	}

	void get(ReadIterator & source, LfgCharacterSearchResultData & target)
	{
		get(source, target.characterId);
		get(source, target.characterName);

		uint16 uShort = 0;
		get(source, uShort);
		target.species = static_cast<SharedCreatureObjectTemplate::Species>(uShort);

		uint8 uChar = 0;
		get(source, uChar);
		target.profession = static_cast<LfgCharacterData::Profession>(uChar);

		get(source, target.level);
		get(source, target.faction);
		get(source, target.guildName);
		get(source, target.guildAbbrev);
		get(source, target.groupId);
		get(source, target.locationPlanet);
		get(source, target.locationRegion);
		get(source, target.locationPlayerCity);
		get(source, target.ctsSourceGalaxy);
		get(source, target.characterInterests);
	}

	void put(ByteStream & target, const LfgCharacterSearchResultData & source)
	{
		put(target, source.characterId);
		put(target, source.characterName);
		put(target, static_cast<uint16>(source.species));
		put(target, static_cast<uint8>(source.profession));
		put(target, source.level);
		put(target, source.faction);
		put(target, source.guildName);
		put(target, source.guildAbbrev);
		put(target, source.groupId);
		put(target, source.locationPlanet);
		put(target, source.locationRegion);
		put(target, source.locationPlayerCity);
		put(target, source.ctsSourceGalaxy);
		put(target, source.characterInterests);
	}
}

// ======================================================================

LfgCharacterData::LfgCharacterData() :
	characterId(),
	characterCreateTime(-1),
	characterAge(-1),
	characterName(),
	characterLowercaseFirstName(),
	species(SharedCreatureObjectTemplate::SP_human),
	gender(SharedCreatureObjectTemplate::GE_other),
	profession(LfgCharacterData::Prof_Unknown),
	pilot(LfgCharacterData::Pilot_None),
	level(0),
	faction(0),
	guildName(),
	guildAbbrev(),
	citizenOfCity(),
	groupId(),
	locationPlanet(),
	locationRegion(),
	locationFactionalPresenceGcwRegion(),
	locationFactionalPresenceGridX(0),
	locationFactionalPresenceGridZ(0),
	locationPlayerCity(),
	ctsSourceGalaxy(),
	searchableByCtsSourceGalaxy(false),
	displayLocationInSearchResults(false),
	anonymous(false),
	active(false),
	characterInterests(),
	sessionActivity(0)
{
}

// ----------------------------------------------------------------------

bool LfgCharacterData::operator== (const LfgCharacterData& rhs) const
{
	return ((characterId == rhs.characterId) &&
	        (characterCreateTime == rhs.characterCreateTime) &&
	        (characterAge == rhs.characterAge) &&
	        (characterName == rhs.characterName) &&
	        (characterLowercaseFirstName == rhs.characterLowercaseFirstName) &&
	        (species == rhs.species) &&
	        (gender == rhs.gender) &&
	        (profession == rhs.profession) &&
	        (pilot == rhs.pilot) &&
	        (level == rhs.level) &&
	        (faction == rhs.faction) &&
	        (guildName == rhs.guildName) &&
	        (guildAbbrev == rhs.guildAbbrev) &&
			(citizenOfCity == rhs.citizenOfCity) &&
	        (groupId == rhs.groupId) &&
	        (locationPlanet == rhs.locationPlanet) &&
	        (locationRegion == rhs.locationRegion) &&
			(locationFactionalPresenceGcwRegion == rhs.locationFactionalPresenceGcwRegion) &&
			(locationFactionalPresenceGridX == rhs.locationFactionalPresenceGridX) &&
			(locationFactionalPresenceGridZ == rhs.locationFactionalPresenceGridZ) &&
			(locationPlayerCity == rhs.locationPlayerCity) &&
			(ctsSourceGalaxy == rhs.ctsSourceGalaxy) &&
			(searchableByCtsSourceGalaxy == rhs.searchableByCtsSourceGalaxy) &&
	        (displayLocationInSearchResults == rhs.displayLocationInSearchResults) &&
	        (anonymous == rhs.anonymous) &&
	        (active == rhs.active) &&
	        (characterInterests == rhs.characterInterests) &&
	        (sessionActivity == rhs.sessionActivity)
	       );
}

// ----------------------------------------------------------------------

bool LfgCharacterData::operator!= (const LfgCharacterData& rhs) const
{
	return !operator==(rhs);
}

// ----------------------------------------------------------------------

LfgCharacterData::Profession LfgCharacterData::convertSkillTemplateToProfession(const std::string & skillTemplate)
{
	if (skillTemplate.find("smuggler_") == 0)
		return LfgCharacterData::Prof_Smuggler;
	else if (skillTemplate.find("bounty_hunter_") == 0)
		return LfgCharacterData::Prof_Bh;
	else if (skillTemplate.find("officer_") == 0)
		return LfgCharacterData::Prof_Officer;
	else if (skillTemplate.find("commando_") == 0)
		return LfgCharacterData::Prof_Commando;
	else if (skillTemplate.find("force_sensitive_") == 0)
		return LfgCharacterData::Prof_Jedi;
	else if (skillTemplate.find("medic_") == 0)
		return LfgCharacterData::Prof_Medic;
	else if (skillTemplate.find("spy_") == 0)
		return LfgCharacterData::Prof_Spy;
	else if (skillTemplate.find("entertainer_") == 0)
		return LfgCharacterData::Prof_Entertainer;
	else if (skillTemplate == "trader_0a")
		return LfgCharacterData::Prof_Trader_Domestics;
	else if (skillTemplate == "trader_0b")
		return LfgCharacterData::Prof_Trader_Structures;
	else if (skillTemplate == "trader_0c")
		return LfgCharacterData::Prof_Trader_Munitions;
	else if (skillTemplate == "trader_0d")
		return LfgCharacterData::Prof_Trader_Engineering;

	return LfgCharacterData::Prof_Unknown;
}

// ----------------------------------------------------------------------

std::string LfgCharacterData::getProfessionDebugString(LfgCharacterData::Profession profession)
{
	std::string output;

	if (profession == LfgCharacterData::Prof_Bh)
		output = "Bounty Hunter";
	else if (profession == LfgCharacterData::Prof_Commando)
		output = "Commando";
	else if (profession == LfgCharacterData::Prof_Entertainer)
		output = "Entertainer";
	else if (profession == LfgCharacterData::Prof_Jedi)
		output = "Jedi";
	else if (profession == LfgCharacterData::Prof_Medic)
		output = "Medic";
	else if (profession == LfgCharacterData::Prof_Officer)
		output = "Officer";
	else if (profession == LfgCharacterData::Prof_Smuggler)
		output = "Smuggler";
	else if (profession == LfgCharacterData::Prof_Spy)
		output = "Spy";
	else if (profession == LfgCharacterData::Prof_Trader_Domestics)
		output = "Trader - Domestics";
	else if (profession == LfgCharacterData::Prof_Trader_Engineering)
		output = "Trader - Engineering";
	else if (profession == LfgCharacterData::Prof_Trader_Munitions)
		output = "Trader - Munitions";
	else if (profession == LfgCharacterData::Prof_Trader_Structures)
		output = "Trader - Structures";
	else if (profession == LfgCharacterData::Prof_NPC)
		output = "NPC";
	else if (profession == LfgCharacterData::Prof_Unknown)
		output = "Unknown";
	else
		output = FormattedString<32>().sprintf("%d", static_cast<int>(profession));

	return output;
}

// ----------------------------------------------------------------------

Unicode::String const & LfgCharacterData::getProfessionDisplayString(LfgCharacterData::Profession profession)
{
	if (s_professionDisplayString.empty())
	{
		s_professionDisplayString.resize(static_cast<size_t>(LfgCharacterData::Profession_MAX));
		s_professionDisplayString[static_cast<size_t>(LfgCharacterData::Prof_Unknown)] = StringId("ui_prof", "unknown").localize();
		s_professionDisplayString[static_cast<size_t>(LfgCharacterData::Prof_Bh)] = StringId("ui_roadmap", "bounty_hunter").localize();
		s_professionDisplayString[static_cast<size_t>(LfgCharacterData::Prof_Commando)] = StringId("ui_roadmap", "commando").localize();
		s_professionDisplayString[static_cast<size_t>(LfgCharacterData::Prof_Entertainer)] = StringId("ui_roadmap", "entertainer").localize();
		s_professionDisplayString[static_cast<size_t>(LfgCharacterData::Prof_Jedi)] = StringId("ui_roadmap", "force_sensitive").localize();
		s_professionDisplayString[static_cast<size_t>(LfgCharacterData::Prof_Medic)] = StringId("ui_roadmap", "medic").localize();
		s_professionDisplayString[static_cast<size_t>(LfgCharacterData::Prof_Officer)] = StringId("ui_roadmap", "officer").localize();
		s_professionDisplayString[static_cast<size_t>(LfgCharacterData::Prof_Smuggler)] = StringId("ui_roadmap", "smuggler").localize();
		s_professionDisplayString[static_cast<size_t>(LfgCharacterData::Prof_Spy)] = StringId("ui_roadmap", "spy").localize();
		s_professionDisplayString[static_cast<size_t>(LfgCharacterData::Prof_Trader_Domestics)] = StringId("ui_roadmap", "trader_0a").localize();
		s_professionDisplayString[static_cast<size_t>(LfgCharacterData::Prof_Trader_Engineering)] = StringId("ui_roadmap", "trader_0d").localize();
		s_professionDisplayString[static_cast<size_t>(LfgCharacterData::Prof_Trader_Munitions)] = StringId("ui_roadmap", "trader_0c").localize();
		s_professionDisplayString[static_cast<size_t>(LfgCharacterData::Prof_Trader_Structures)] = StringId("ui_roadmap", "trader_0b").localize();
		s_professionDisplayString[static_cast<size_t>(LfgCharacterData::Prof_NPC)] = StringId("ui_prof", "npc").localize();
	}

	static Unicode::String empty;

	if ((static_cast<int>(profession) >= 0) && (static_cast<int>(profession) < s_professionDisplayString.size()))
		return s_professionDisplayString[static_cast<size_t>(profession)];

	return empty;
}

// ----------------------------------------------------------------------

std::string LfgCharacterData::getDebugString() const
{
	if (s_activities.empty())
		initializeActivitiesList();

	std::string output = Unicode::wideToNarrow(characterName);
	output += " (";
	output += characterId.getValueString();
	output += ")";

	output += "\nFirst Name:      ";
	output += characterLowercaseFirstName;

	output += "\nCreate Time:     ";
	if (characterCreateTime > 0)
		output += CalendarTime::convertEpochToTimeStringLocal(static_cast<time_t>(characterCreateTime)) + ", " + CalendarTime::convertEpochToTimeStringGMT(static_cast<time_t>(characterCreateTime));
	else
		output += "N/A";

	output += "\nAge:             ";
	output += FormattedString<128>().sprintf("%d days", characterAge);

	output += "\nCTS Source Galaxy: ";
	if (ctsSourceGalaxy.empty())
	{
		output += "(none)";
	}
	else
	{
		bool first = true;
		for (std::set<std::string>::const_iterator iter = ctsSourceGalaxy.begin(); iter != ctsSourceGalaxy.end(); ++iter)
		{
			if (first)
				first = false;
			else
				output += ", ";

			output += *iter;
		}
	}

	output += "\nSearchable By CTS Source Galaxy: ";
	output += (searchableByCtsSourceGalaxy ? "yes" : "no");

	output += "\nDisplay Location In Search Results: ";
	output += (displayLocationInSearchResults ? "yes" : "no");

	output += "\nAnonymous:       ";
	output += (anonymous ? "yes" : "no");

	output += "\nActive:          ";
	output += (active ? "yes" : "no");

	output += "\nLocation:        ";

	if (!locationPlayerCity.empty())
	{
		output += locationPlayerCity;
		output += ", ";
	}

	output += locationPlanet;

	if (!locationRegion.empty())
	{
		output += ",";
		output += locationRegion;
	}

	if (!locationFactionalPresenceGcwRegion.empty())
	{
		output += ",";
		output += locationFactionalPresenceGcwRegion;

		if (::strncmp("space_", locationPlanet.c_str(), 6))
			output += FormattedString<128>().sprintf(" (%d, %d) [grid (%d, %d) -> (%d, %d)]", locationFactionalPresenceGridX, locationFactionalPresenceGridZ, (locationFactionalPresenceGridX - (PvpData::getGcwFactionalPresenceGridSize() / 2)), (locationFactionalPresenceGridZ - (PvpData::getGcwFactionalPresenceGridSize() / 2)), (locationFactionalPresenceGridX + (PvpData::getGcwFactionalPresenceGridSize() / 2)), (locationFactionalPresenceGridZ + (PvpData::getGcwFactionalPresenceGridSize() / 2)));
	}

	output += "\nSpecies:         ";
	if (species == SharedCreatureObjectTemplate::SP_bothan)
		output += "Bothan";
	else if (species == SharedCreatureObjectTemplate::SP_human)
		output += "Human";
	else if (species == SharedCreatureObjectTemplate::SP_ithorian)
		output += "Ithorian";
	else if (species == SharedCreatureObjectTemplate::SP_monCalamari)
		output += "Mon Calamari";
	else if (species == SharedCreatureObjectTemplate::SP_rodian)
		output += "Rodian";
	else if (species == SharedCreatureObjectTemplate::SP_sullustan)
		output += "Sullustan";
	else if (species == SharedCreatureObjectTemplate::SP_trandoshan)
		output += "Trandoshan";
	else if (species == SharedCreatureObjectTemplate::SP_twilek)
		output += "Twilek";
	else if (species == SharedCreatureObjectTemplate::SP_wookiee)
		output += "Wookiee";
	else if (species == SharedCreatureObjectTemplate::SP_zabrak)
		output += "Zabrak";
	else
		output += FormattedString<32>().sprintf("%d", static_cast<int>(species));
 
	output += "\nGender:          ";
	if (gender == SharedCreatureObjectTemplate::GE_male)
		output += "Male";
	else if (gender == SharedCreatureObjectTemplate::GE_female)
		output += "Female";
	else if (gender == SharedCreatureObjectTemplate::GE_other)
		output += "Other";
	else
		output += FormattedString<32>().sprintf("%d", static_cast<int>(gender));

	output += "\nProfession:      ";
	output += getProfessionDebugString(profession);

	output += FormattedString<64>().sprintf("\nLevel:           %d", level);

	output += "\nFaction:         ";
	if (PvpData::isImperialFactionId(faction))
		output += "Imperial";
	else if (PvpData::isRebelFactionId(faction))
		output += "Rebel";
	else if (PvpData::isNeutralFactionId(faction))
		output += "Neutral";
	else
		output += FormattedString<32>().sprintf("%lu", faction);

	output += "\nPilot:           ";
	if (pilot == LfgCharacterData::Pilot_Imperial)
		output += "Imperial";
	else if (pilot == LfgCharacterData::Pilot_Rebel)
		output += "Rebel";
	else if (pilot == LfgCharacterData::Pilot_Neutral)
		output += "Neutral";
	else if (pilot == LfgCharacterData::Pilot_None)
		output += "(not a pilot)";
	else
		output += FormattedString<32>().sprintf("%d", static_cast<int>(pilot));

	output += "\nGuild:           ";
	if (guildName.empty())
	{
		output += "(unguilded)";
	}
	else
	{
		output += guildName;
		output += " (";
		output += guildAbbrev;
		output += ")";
	}

	output += "\nCitizen Of:      ";
	if (!citizenOfCity.empty())
	{
		output += citizenOfCity;
	}

	output += "\nGroup Id:        ";
	output += groupId.getValueString();

	output += "\nActivities:      ";
	std::string activities;
	for (std::map<uint32, std::string>::const_iterator iterActivities = s_activities.begin(); iterActivities != s_activities.end(); ++iterActivities)
	{
		if ((sessionActivity & iterActivities->first) == iterActivities->first)
		{
			std::string activity = iterActivities->second;
			std::string::size_type const dotPos = activity.find('.');
			if ((dotPos != std::string::npos) && ((dotPos + 1) < activity.size()))
				activity = activity.substr(dotPos + 1);

			if (!activity.empty())
			{
				if (!activities.empty())
					activities += ", ";

				activities += activity;
			}
		}
	}

	if (activities.empty())
		activities = "None";

	output += activities;

	output += "\nInterests:       ";
	std::string sInterests = LfgDataTable::getSetLfgNodeDebugString(characterInterests, false);
	if (sInterests.empty())
	{
		output += "(none specified)\n";
	}
	else
	{
		output += "\n";
		output += sInterests;
	}

	return output;
}

// ----------------------------------------------------------------------

std::map<std::string, int> const & LfgCharacterData::calculateStatistics(std::map<NetworkId, LfgCharacterData> const & connectedCharacterLfgData)
{
	if (s_activities.empty())
		initializeActivitiesList();

	static std::map<std::string, int> statistics;
	if (statistics.empty())
	{
		// all possible statistics ***SHOULD*** be inserted/initialized here
		// ***DO NOT*** insert new statistics into the statistics map outside
		// of this initialization block; outside of this initialization block,
		// the only change to the statistics map should be to update the count
		// for individual statistics;

		// searchable/anonymous
		statistics[s_stat_searchable_section] = -1;
		statistics[s_stat_anonymous] = 0;
		statistics[s_stat_searchable] = 0;

		// active/inactive
		statistics[s_stat_active_section] = -1;
		statistics[s_stat_active] = 0;
		statistics[s_stat_inactive] = 0;

		// grouped/ungrouped
		statistics[s_stat_grouped_section] = -1;
		statistics[s_stat_grouped] = 0;
		statistics[s_stat_ungrouped] = 0;

		// guilded/unguilded
		statistics[s_stat_guilded_section] = -1;
		statistics[s_stat_guilded] = 0;
		statistics[s_stat_unguilded] = 0;

		// searchableByCtsSourceGalaxy
		statistics[s_stat_searchablebyctssourcegalaxy_section] = -1;
		statistics[s_stat_searchablebyctssourcegalaxy_yes] = 0;
		statistics[s_stat_searchablebyctssourcegalaxy_no] = 0;

		// displayLocationInSearchResults
		statistics[s_stat_displayLocation_section] = -1;
		statistics[s_stat_displayLocation] = 0;
		statistics[s_stat_hideLocation] = 0;

		// gender
		statistics[s_stat_gender_section] = -1;
		statistics[s_stat_female] = 0;
		statistics[s_stat_male] = 0;

		// faction
		statistics[s_stat_faction_section] = -1;
		statistics[s_stat_faction_imperial] = 0;
		statistics[s_stat_faction_neutral] = 0;
		statistics[s_stat_faction_rebel] = 0;

		// pilot
		statistics[s_stat_pilot_section] = -1;
		statistics[s_stat_pilot_imperial] = 0;
		statistics[s_stat_pilot_neutral] = 0;
		statistics[s_stat_pilot_none] = 0;
		statistics[s_stat_pilot_rebel] = 0;

		// species
		statistics[s_stat_species_section] = -1;
		statistics[s_stat_bothan] = 0;
		statistics[s_stat_human] = 0;
		statistics[s_stat_ithorian] = 0;
		statistics[s_stat_monCalamari] = 0;
		statistics[s_stat_rodian] = 0;
		statistics[s_stat_sullustan] = 0;
		statistics[s_stat_trandoshan] = 0;
		statistics[s_stat_twilek] = 0;
		statistics[s_stat_wookiee] = 0;
		statistics[s_stat_zabrak] = 0;

		// species by gender
		statistics[s_stat_species_gender_section] = -1;
		statistics[s_stat_bothan_m] = 0;
		statistics[s_stat_bothan_f] = 0;
		statistics[s_stat_human_m] = 0;
		statistics[s_stat_human_f] = 0;
		statistics[s_stat_ithorian_m] = 0;
		statistics[s_stat_ithorian_f] = 0;
		statistics[s_stat_monCalamari_m] = 0;
		statistics[s_stat_monCalamari_f] = 0;
		statistics[s_stat_rodian_m] = 0;
		statistics[s_stat_rodian_f] = 0;
		statistics[s_stat_sullustan_m] = 0;
		statistics[s_stat_sullustan_f] = 0;
		statistics[s_stat_trandoshan_m] = 0;
		statistics[s_stat_trandoshan_f] = 0;
		statistics[s_stat_twilek_m] = 0;
		statistics[s_stat_twilek_f] = 0;
		statistics[s_stat_wookiee_m] = 0;
		statistics[s_stat_wookiee_f] = 0;
		statistics[s_stat_zabrak_m] = 0;
		statistics[s_stat_zabrak_f] = 0;

		// profession
		statistics[s_stat_profession_section] = -1;
		statistics[s_stat_bountyHunter] = 0;
		statistics[s_stat_commando] = 0;
		statistics[s_stat_entertainer] = 0;
		statistics[s_stat_jedi] = 0;
		statistics[s_stat_medic] = 0;
		statistics[s_stat_officer] = 0;
		statistics[s_stat_smuggler] = 0;
		statistics[s_stat_spy] = 0;
		statistics[s_stat_traderDomestics] = 0;
		statistics[s_stat_traderEngineering] = 0;
		statistics[s_stat_traderMunitions] = 0;
		statistics[s_stat_traderStructures] = 0;

		// profession by gender
		statistics[s_stat_profession_gender_section] = -1;
		statistics[s_stat_bountyHunter_m] = 0;
		statistics[s_stat_bountyHunter_f] = 0;
		statistics[s_stat_commando_m] = 0;
		statistics[s_stat_commando_f] = 0;
		statistics[s_stat_entertainer_m] = 0;
		statistics[s_stat_entertainer_f] = 0;
		statistics[s_stat_jedi_m] = 0;
		statistics[s_stat_jedi_f] = 0;
		statistics[s_stat_medic_m] = 0;
		statistics[s_stat_medic_f] = 0;
		statistics[s_stat_officer_m] = 0;
		statistics[s_stat_officer_f] = 0;
		statistics[s_stat_smuggler_m] = 0;
		statistics[s_stat_smuggler_f] = 0;
		statistics[s_stat_spy_m] = 0;
		statistics[s_stat_spy_f] = 0;
		statistics[s_stat_traderDomestics_m] = 0;
		statistics[s_stat_traderDomestics_f] = 0;
		statistics[s_stat_traderEngineering_m] = 0;
		statistics[s_stat_traderEngineering_f] = 0;
		statistics[s_stat_traderMunitions_m] = 0;
		statistics[s_stat_traderMunitions_f] = 0;
		statistics[s_stat_traderStructures_m] = 0;
		statistics[s_stat_traderStructures_f] = 0;

		// level
		statistics[s_stat_level_section] = -1;
		statistics[s_stat_level_10] = 0;
		statistics[s_stat_level_10_imperial] = 0;
		statistics[s_stat_level_10_neutral] = 0;
		statistics[s_stat_level_10_rebel] = 0;
		statistics[s_stat_level_20] = 0;
		statistics[s_stat_level_20_imperial] = 0;
		statistics[s_stat_level_20_neutral] = 0;
		statistics[s_stat_level_20_rebel] = 0;
		statistics[s_stat_level_30] = 0;
		statistics[s_stat_level_30_imperial] = 0;
		statistics[s_stat_level_30_neutral] = 0;
		statistics[s_stat_level_30_rebel] = 0;
		statistics[s_stat_level_40] = 0;
		statistics[s_stat_level_40_imperial] = 0;
		statistics[s_stat_level_40_neutral] = 0;
		statistics[s_stat_level_40_rebel] = 0;
		statistics[s_stat_level_50] = 0;
		statistics[s_stat_level_50_imperial] = 0;
		statistics[s_stat_level_50_neutral] = 0;
		statistics[s_stat_level_50_rebel] = 0;
		statistics[s_stat_level_60] = 0;
		statistics[s_stat_level_60_imperial] = 0;
		statistics[s_stat_level_60_neutral] = 0;
		statistics[s_stat_level_60_rebel] = 0;
		statistics[s_stat_level_70] = 0;
		statistics[s_stat_level_70_imperial] = 0;
		statistics[s_stat_level_70_neutral] = 0;
		statistics[s_stat_level_70_rebel] = 0;
		statistics[s_stat_level_80] = 0;
		statistics[s_stat_level_80_imperial] = 0;
		statistics[s_stat_level_80_neutral] = 0;
		statistics[s_stat_level_80_rebel] = 0;
		statistics[s_stat_level_89] = 0;
		statistics[s_stat_level_89_imperial] = 0;
		statistics[s_stat_level_89_neutral] = 0;
		statistics[s_stat_level_89_rebel] = 0;
		statistics[s_stat_level_90] = 0;
		statistics[s_stat_level_90_imperial] = 0;
		statistics[s_stat_level_90_neutral] = 0;
		statistics[s_stat_level_90_rebel] = 0;

		// location planet
		statistics[s_stat_location_section] = -1;
		for (int i = 0; !s_all_planets[i].empty(); ++i)
			statistics[s_stat_location_section + "." + s_all_planets[i]] = 0;

		// activities
		statistics[s_stat_activities_section] = -1;
		for (std::map<uint32, std::string>::const_iterator iterActivities = s_activities.begin(); iterActivities != s_activities.end(); ++iterActivities)
			statistics[iterActivities->second] = 0;

		// character create time
		statistics[s_stat_character_create_time_section] = -1;
		statistics[s_stat_character_create_time_001day] = 0;
		statistics[s_stat_character_create_time_002day] = 0;
		statistics[s_stat_character_create_time_003day] = 0;
		statistics[s_stat_character_create_time_004day] = 0;
		statistics[s_stat_character_create_time_005day] = 0;
		statistics[s_stat_character_create_time_006day] = 0;
		statistics[s_stat_character_create_time_007day] = 0;
		statistics[s_stat_character_create_time_014day] = 0;
		statistics[s_stat_character_create_time_021day] = 0;
		statistics[s_stat_character_create_time_030day] = 0;
		statistics[s_stat_character_create_time_060day] = 0;
		statistics[s_stat_character_create_time_090day] = 0;
		statistics[s_stat_character_create_time_180day] = 0;
		statistics[s_stat_character_create_time_270day] = 0;
		statistics[s_stat_character_create_time_1year] = 0;
		statistics[s_stat_character_create_time_2year] = 0;
		statistics[s_stat_character_create_time_3year] = 0;

		// character age
		statistics[s_stat_character_age_section] = -1;
		statistics[s_stat_character_age_001day] = 0;
		statistics[s_stat_character_age_002day] = 0;
		statistics[s_stat_character_age_003day] = 0;
		statistics[s_stat_character_age_004day] = 0;
		statistics[s_stat_character_age_005day] = 0;
		statistics[s_stat_character_age_006day] = 0;
		statistics[s_stat_character_age_007day] = 0;
		statistics[s_stat_character_age_014day] = 0;
		statistics[s_stat_character_age_021day] = 0;
		statistics[s_stat_character_age_030day] = 0;
		statistics[s_stat_character_age_060day] = 0;
		statistics[s_stat_character_age_090day] = 0;
		statistics[s_stat_character_age_180day] = 0;
		statistics[s_stat_character_age_270day] = 0;
		statistics[s_stat_character_age_1year] = 0;
		statistics[s_stat_character_age_2year] = 0;
		statistics[s_stat_character_age_3year] = 0;
	}

	// character create time
	static std::map<int, std::pair<std::string, int> > createTimeStatistics;
	if (createTimeStatistics.empty())
	{
		createTimeStatistics[(60 * 60 * 24 * 1)]    = std::make_pair(s_stat_character_create_time_001day, 0);
		createTimeStatistics[(60 * 60 * 24 * 2)]    = std::make_pair(s_stat_character_create_time_002day, 0);
		createTimeStatistics[(60 * 60 * 24 * 3)]    = std::make_pair(s_stat_character_create_time_003day, 0);
		createTimeStatistics[(60 * 60 * 24 * 4)]    = std::make_pair(s_stat_character_create_time_004day, 0);
		createTimeStatistics[(60 * 60 * 24 * 5)]    = std::make_pair(s_stat_character_create_time_005day, 0);
		createTimeStatistics[(60 * 60 * 24 * 6)]    = std::make_pair(s_stat_character_create_time_006day, 0);
		createTimeStatistics[(60 * 60 * 24 * 7)]    = std::make_pair(s_stat_character_create_time_007day, 0);
		createTimeStatistics[(60 * 60 * 24 * 14)]   = std::make_pair(s_stat_character_create_time_014day, 0);
		createTimeStatistics[(60 * 60 * 24 * 21)]   = std::make_pair(s_stat_character_create_time_021day, 0);
		createTimeStatistics[(60 * 60 * 24 * 30)]   = std::make_pair(s_stat_character_create_time_030day, 0);
		createTimeStatistics[(60 * 60 * 24 * 60)]   = std::make_pair(s_stat_character_create_time_060day, 0);
		createTimeStatistics[(60 * 60 * 24 * 90)]   = std::make_pair(s_stat_character_create_time_090day, 0);
		createTimeStatistics[(60 * 60 * 24 * 180)]  = std::make_pair(s_stat_character_create_time_180day, 0);
		createTimeStatistics[(60 * 60 * 24 * 270)]  = std::make_pair(s_stat_character_create_time_270day, 0);
		createTimeStatistics[(60 * 60 * 24 * 365)]  = std::make_pair(s_stat_character_create_time_1year, 0);
		createTimeStatistics[(60 * 60 * 24 * 730)]  = std::make_pair(s_stat_character_create_time_2year, 0);
		createTimeStatistics[(60 * 60 * 24 * 1095)] = std::make_pair(s_stat_character_create_time_3year, 0);
	}

	// character age
	static std::map<int, std::pair<std::string, int> > ageStatistics;
	if (ageStatistics.empty())
	{
		ageStatistics[1]    = std::make_pair(s_stat_character_age_001day, 0);
		ageStatistics[2]    = std::make_pair(s_stat_character_age_002day, 0);
		ageStatistics[3]    = std::make_pair(s_stat_character_age_003day, 0);
		ageStatistics[4]    = std::make_pair(s_stat_character_age_004day, 0);
		ageStatistics[5]    = std::make_pair(s_stat_character_age_005day, 0);
		ageStatistics[6]    = std::make_pair(s_stat_character_age_006day, 0);
		ageStatistics[7]    = std::make_pair(s_stat_character_age_007day, 0);
		ageStatistics[14]   = std::make_pair(s_stat_character_age_014day, 0);
		ageStatistics[21]   = std::make_pair(s_stat_character_age_021day, 0);
		ageStatistics[30]   = std::make_pair(s_stat_character_age_030day, 0);
		ageStatistics[60]   = std::make_pair(s_stat_character_age_060day, 0);
		ageStatistics[90]   = std::make_pair(s_stat_character_age_090day, 0);
		ageStatistics[180]  = std::make_pair(s_stat_character_age_180day, 0);
		ageStatistics[270]  = std::make_pair(s_stat_character_age_270day, 0);
		ageStatistics[365]  = std::make_pair(s_stat_character_age_1year, 0);
		ageStatistics[730]  = std::make_pair(s_stat_character_age_2year, 0);
		ageStatistics[1095] = std::make_pair(s_stat_character_age_3year, 0);
	}

	// set all statistics count to 0
	{
		for (std::map<std::string, int>::iterator iter = statistics.begin(); iter != statistics.end(); ++iter)
		{
			if (iter->second > 0)
				iter->second = 0;
		}

		for (std::map<int, std::pair<std::string, int> >::iterator iterCreateTimeStatistics = createTimeStatistics.begin(); iterCreateTimeStatistics != createTimeStatistics.end(); ++iterCreateTimeStatistics)
		{
			iterCreateTimeStatistics->second.second = 0;
		}

		for (std::map<int, std::pair<std::string, int> >::iterator iterAgeStatistics = ageStatistics.begin(); iterAgeStatistics != ageStatistics.end(); ++iterAgeStatistics)
		{
			iterAgeStatistics->second.second = 0;
		}
	}

	int32 const timeNow = static_cast<int32>(::time(nullptr));
	for (std::map<NetworkId, LfgCharacterData>::const_iterator iterLfgData = connectedCharacterLfgData.begin(); iterLfgData != connectedCharacterLfgData.end(); ++iterLfgData)
	{
		// searchable/anonymous
		if (iterLfgData->second.anonymous)
			++(statistics[s_stat_anonymous]);
		else
			++(statistics[s_stat_searchable]);

		// active/inactive
		if (iterLfgData->second.active)
			++(statistics[s_stat_active]);
		else
			++(statistics[s_stat_inactive]);

		// grouped/ungrouped
		if (iterLfgData->second.groupId.isValid())
			++(statistics[s_stat_grouped]);
		else
			++(statistics[s_stat_ungrouped]);

		// guilded/unguilded
		if (!iterLfgData->second.guildName.empty())
			++(statistics[s_stat_guilded]);
		else
			++(statistics[s_stat_unguilded]);

		// searchableByCtsSourceGalaxy
		if (iterLfgData->second.searchableByCtsSourceGalaxy)
			++(statistics[s_stat_searchablebyctssourcegalaxy_yes]);
		else
			++(statistics[s_stat_searchablebyctssourcegalaxy_no]);

		// displayLocationInSearchResults
		if (iterLfgData->second.displayLocationInSearchResults)
			++(statistics[s_stat_displayLocation]);
		else
			++(statistics[s_stat_hideLocation]);

		// gender
		if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_male)
			++(statistics[s_stat_male]);
		else if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_female)
			++(statistics[s_stat_female]);

		// faction
		if (PvpData::isImperialFactionId(iterLfgData->second.faction))
			++(statistics[s_stat_faction_imperial]);
		else if (PvpData::isRebelFactionId(iterLfgData->second.faction))
			++(statistics[s_stat_faction_rebel]);
		else if (PvpData::isNeutralFactionId(iterLfgData->second.faction))
			++(statistics[s_stat_faction_neutral]);

		// pilot
		if (iterLfgData->second.pilot == LfgCharacterData::Pilot_Imperial)
			++(statistics[s_stat_pilot_imperial]);
		else if (iterLfgData->second.pilot == LfgCharacterData::Pilot_Rebel)
			++(statistics[s_stat_pilot_rebel]);
		else if (iterLfgData->second.pilot == LfgCharacterData::Pilot_Neutral)
			++(statistics[s_stat_pilot_neutral]);
		else if (iterLfgData->second.pilot == LfgCharacterData::Pilot_None)
			++(statistics[s_stat_pilot_none]);

		// species and species by gender
		if (iterLfgData->second.species == SharedCreatureObjectTemplate::SP_bothan)
		{
			++(statistics[s_stat_bothan]);

			if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_male)
				++(statistics[s_stat_bothan_m]);
			else if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_female)
				++(statistics[s_stat_bothan_f]);
		}
		else if (iterLfgData->second.species == SharedCreatureObjectTemplate::SP_human)
		{
			++(statistics[s_stat_human]);

			if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_male)
				++(statistics[s_stat_human_m]);
			else if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_female)
				++(statistics[s_stat_human_f]);
		}
		else if (iterLfgData->second.species == SharedCreatureObjectTemplate::SP_ithorian)
		{
			++(statistics[s_stat_ithorian]);

			if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_male)
				++(statistics[s_stat_ithorian_m]);
			else if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_female)
				++(statistics[s_stat_ithorian_f]);
		}
		else if (iterLfgData->second.species == SharedCreatureObjectTemplate::SP_monCalamari)
		{
			++(statistics[s_stat_monCalamari]);

			if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_male)
				++(statistics[s_stat_monCalamari_m]);
			else if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_female)
				++(statistics[s_stat_monCalamari_f]);
		}
		else if (iterLfgData->second.species == SharedCreatureObjectTemplate::SP_rodian)
		{
			++(statistics[s_stat_rodian]);

			if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_male)
				++(statistics[s_stat_rodian_m]);
			else if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_female)
				++(statistics[s_stat_rodian_f]);
		}
		else if (iterLfgData->second.species == SharedCreatureObjectTemplate::SP_sullustan)
		{
			++(statistics[s_stat_sullustan]);

			if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_male)
				++(statistics[s_stat_sullustan_m]);
			else if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_female)
				++(statistics[s_stat_sullustan_f]);
		}
		else if (iterLfgData->second.species == SharedCreatureObjectTemplate::SP_trandoshan)
		{
			++(statistics[s_stat_trandoshan]);

			if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_male)
				++(statistics[s_stat_trandoshan_m]);
			else if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_female)
				++(statistics[s_stat_trandoshan_f]);
		}
		else if (iterLfgData->second.species == SharedCreatureObjectTemplate::SP_twilek)
		{
			++(statistics[s_stat_twilek]);

			if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_male)
				++(statistics[s_stat_twilek_m]);
			else if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_female)
				++(statistics[s_stat_twilek_f]);
		}
		else if (iterLfgData->second.species == SharedCreatureObjectTemplate::SP_wookiee)
		{
			++(statistics[s_stat_wookiee]);

			if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_male)
				++(statistics[s_stat_wookiee_m]);
			else if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_female)
				++(statistics[s_stat_wookiee_f]);
		}
		else if (iterLfgData->second.species == SharedCreatureObjectTemplate::SP_zabrak)
		{
			++(statistics[s_stat_zabrak]);

			if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_male)
				++(statistics[s_stat_zabrak_m]);
			else if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_female)
				++(statistics[s_stat_zabrak_f]);
		}

		// profession and profession by gender
		if (iterLfgData->second.profession == LfgCharacterData::Prof_Bh)
		{
			++(statistics[s_stat_bountyHunter]);

			if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_male)
				++(statistics[s_stat_bountyHunter_m]);
			else if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_female)
				++(statistics[s_stat_bountyHunter_f]);
		}
		else if (iterLfgData->second.profession == LfgCharacterData::Prof_Commando)
		{
			++(statistics[s_stat_commando]);

			if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_male)
				++(statistics[s_stat_commando_m]);
			else if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_female)
				++(statistics[s_stat_commando_f]);
		}
		else if (iterLfgData->second.profession == LfgCharacterData::Prof_Entertainer)
		{
			++(statistics[s_stat_entertainer]);

			if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_male)
				++(statistics[s_stat_entertainer_m]);
			else if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_female)
				++(statistics[s_stat_entertainer_f]);
		}
		else if (iterLfgData->second.profession == LfgCharacterData::Prof_Jedi)
		{
			++(statistics[s_stat_jedi]);

			if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_male)
				++(statistics[s_stat_jedi_m]);
			else if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_female)
				++(statistics[s_stat_jedi_f]);
		}
		else if (iterLfgData->second.profession == LfgCharacterData::Prof_Medic)
		{
			++(statistics[s_stat_medic]);

			if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_male)
				++(statistics[s_stat_medic_m]);
			else if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_female)
				++(statistics[s_stat_medic_f]);
		}
		else if (iterLfgData->second.profession == LfgCharacterData::Prof_Officer)
		{
			++(statistics[s_stat_officer]);

			if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_male)
				++(statistics[s_stat_officer_m]);
			else if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_female)
				++(statistics[s_stat_officer_f]);
		}
		else if (iterLfgData->second.profession == LfgCharacterData::Prof_Smuggler)
		{
			++(statistics[s_stat_smuggler]);

			if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_male)
				++(statistics[s_stat_smuggler_m]);
			else if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_female)
				++(statistics[s_stat_smuggler_f]);
		}
		else if (iterLfgData->second.profession == LfgCharacterData::Prof_Spy)
		{
			++(statistics[s_stat_spy]);

			if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_male)
				++(statistics[s_stat_spy_m]);
			else if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_female)
				++(statistics[s_stat_spy_f]);
		}
		else if (iterLfgData->second.profession == LfgCharacterData::Prof_Trader_Domestics)
		{
			++(statistics[s_stat_traderDomestics]);

			if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_male)
				++(statistics[s_stat_traderDomestics_m]);
			else if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_female)
				++(statistics[s_stat_traderDomestics_f]);
		}
		else if (iterLfgData->second.profession == LfgCharacterData::Prof_Trader_Engineering)
		{
			++(statistics[s_stat_traderEngineering]);

			if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_male)
				++(statistics[s_stat_traderEngineering_m]);
			else if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_female)
				++(statistics[s_stat_traderEngineering_f]);
		}
		else if (iterLfgData->second.profession == LfgCharacterData::Prof_Trader_Munitions)
		{
			++(statistics[s_stat_traderMunitions]);

			if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_male)
				++(statistics[s_stat_traderMunitions_m]);
			else if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_female)
				++(statistics[s_stat_traderMunitions_f]);
		}
		else if (iterLfgData->second.profession == LfgCharacterData::Prof_Trader_Structures)
		{
			++(statistics[s_stat_traderStructures]);

			if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_male)
				++(statistics[s_stat_traderStructures_m]);
			else if (iterLfgData->second.gender == SharedCreatureObjectTemplate::GE_female)
				++(statistics[s_stat_traderStructures_f]);
		}

		// level
		if (iterLfgData->second.level <= 10)
		{
			++(statistics[s_stat_level_10]);

			// faction
			if (PvpData::isImperialFactionId(iterLfgData->second.faction))
				++(statistics[s_stat_level_10_imperial]);
			else if (PvpData::isRebelFactionId(iterLfgData->second.faction))
				++(statistics[s_stat_level_10_rebel]);
			else if (PvpData::isNeutralFactionId(iterLfgData->second.faction))
				++(statistics[s_stat_level_10_neutral]);
		}
		else if (iterLfgData->second.level <= 20)
		{
			++(statistics[s_stat_level_20]);

			// faction
			if (PvpData::isImperialFactionId(iterLfgData->second.faction))
				++(statistics[s_stat_level_20_imperial]);
			else if (PvpData::isRebelFactionId(iterLfgData->second.faction))
				++(statistics[s_stat_level_20_rebel]);
			else if (PvpData::isNeutralFactionId(iterLfgData->second.faction))
				++(statistics[s_stat_level_20_neutral]);
		}
		else if (iterLfgData->second.level <= 30)
		{
			++(statistics[s_stat_level_30]);

			// faction
			if (PvpData::isImperialFactionId(iterLfgData->second.faction))
				++(statistics[s_stat_level_30_imperial]);
			else if (PvpData::isRebelFactionId(iterLfgData->second.faction))
				++(statistics[s_stat_level_30_rebel]);
			else if (PvpData::isNeutralFactionId(iterLfgData->second.faction))
				++(statistics[s_stat_level_30_neutral]);
		}
		else if (iterLfgData->second.level <= 40)
		{
			++(statistics[s_stat_level_40]);

			// faction
			if (PvpData::isImperialFactionId(iterLfgData->second.faction))
				++(statistics[s_stat_level_40_imperial]);
			else if (PvpData::isRebelFactionId(iterLfgData->second.faction))
				++(statistics[s_stat_level_40_rebel]);
			else if (PvpData::isNeutralFactionId(iterLfgData->second.faction))
				++(statistics[s_stat_level_40_neutral]);
		}
		else if (iterLfgData->second.level <= 50)
		{
			++(statistics[s_stat_level_50]);

			// faction
			if (PvpData::isImperialFactionId(iterLfgData->second.faction))
				++(statistics[s_stat_level_50_imperial]);
			else if (PvpData::isRebelFactionId(iterLfgData->second.faction))
				++(statistics[s_stat_level_50_rebel]);
			else if (PvpData::isNeutralFactionId(iterLfgData->second.faction))
				++(statistics[s_stat_level_50_neutral]);
		}
		else if (iterLfgData->second.level <= 60)
		{
			++(statistics[s_stat_level_60]);

			// faction
			if (PvpData::isImperialFactionId(iterLfgData->second.faction))
				++(statistics[s_stat_level_60_imperial]);
			else if (PvpData::isRebelFactionId(iterLfgData->second.faction))
				++(statistics[s_stat_level_60_rebel]);
			else if (PvpData::isNeutralFactionId(iterLfgData->second.faction))
				++(statistics[s_stat_level_60_neutral]);
		}
		else if (iterLfgData->second.level <= 70)
		{
			++(statistics[s_stat_level_70]);

			// faction
			if (PvpData::isImperialFactionId(iterLfgData->second.faction))
				++(statistics[s_stat_level_70_imperial]);
			else if (PvpData::isRebelFactionId(iterLfgData->second.faction))
				++(statistics[s_stat_level_70_rebel]);
			else if (PvpData::isNeutralFactionId(iterLfgData->second.faction))
				++(statistics[s_stat_level_70_neutral]);
		}
		else if (iterLfgData->second.level <= 80)
		{
			++(statistics[s_stat_level_80]);

			// faction
			if (PvpData::isImperialFactionId(iterLfgData->second.faction))
				++(statistics[s_stat_level_80_imperial]);
			else if (PvpData::isRebelFactionId(iterLfgData->second.faction))
				++(statistics[s_stat_level_80_rebel]);
			else if (PvpData::isNeutralFactionId(iterLfgData->second.faction))
				++(statistics[s_stat_level_80_neutral]);
		}
		else if (iterLfgData->second.level <= 89)
		{
			++(statistics[s_stat_level_89]);

			// faction
			if (PvpData::isImperialFactionId(iterLfgData->second.faction))
				++(statistics[s_stat_level_89_imperial]);
			else if (PvpData::isRebelFactionId(iterLfgData->second.faction))
				++(statistics[s_stat_level_89_rebel]);
			else if (PvpData::isNeutralFactionId(iterLfgData->second.faction))
				++(statistics[s_stat_level_89_neutral]);
		}
		else
		{
			++(statistics[s_stat_level_90]);

			// faction
			if (PvpData::isImperialFactionId(iterLfgData->second.faction))
				++(statistics[s_stat_level_90_imperial]);
			else if (PvpData::isRebelFactionId(iterLfgData->second.faction))
				++(statistics[s_stat_level_90_rebel]);
			else if (PvpData::isNeutralFactionId(iterLfgData->second.faction))
				++(statistics[s_stat_level_90_neutral]);
		}

		// location planet
		if (!iterLfgData->second.locationPlanet.empty())
			++(statistics[s_stat_location_section + "." + iterLfgData->second.locationPlanet]);

		// activities
		for (std::map<uint32, std::string>::const_iterator iterActivities = s_activities.begin(); iterActivities != s_activities.end(); ++iterActivities)
		{
			if ((iterLfgData->second.sessionActivity & iterActivities->first) == iterActivities->first)
				++(statistics[iterActivities->second]);
		}

		// character create time
		if (iterLfgData->second.characterCreateTime > 0)
		{
			int32 const createTimeSecondsAgo = std::max(static_cast<int32>(0), (timeNow - iterLfgData->second.characterCreateTime));

			std::map<int, std::pair<std::string, int> >::iterator iterFind = createTimeStatistics.lower_bound(createTimeSecondsAgo);
			if (iterFind != createTimeStatistics.end())
				++(iterFind->second.second);
		}

		// character age
		{
			std::map<int, std::pair<std::string, int> >::iterator iterFind = ageStatistics.lower_bound(iterLfgData->second.characterAge);
			if (iterFind != ageStatistics.end())
				++(iterFind->second.second);
		}
	}

	// tabulate character create time statistics
	{
		std::map<int, std::pair<std::string, int> >::iterator iter = createTimeStatistics.begin();
		std::map<int, std::pair<std::string, int> >::iterator previousIter = createTimeStatistics.begin();

		for (; iter != createTimeStatistics.end(); ++iter)
		{
			if (iter != previousIter)
				iter->second.second += previousIter->second.second;

			previousIter = iter;
		}
	}

	// report character create time statistics
	{
		for (std::map<int, std::pair<std::string, int> >::const_iterator iterCreateTimeStatistics = createTimeStatistics.begin(); iterCreateTimeStatistics != createTimeStatistics.end(); ++iterCreateTimeStatistics)
		{
			std::map<std::string, int>::iterator iterStatistics = statistics.find(iterCreateTimeStatistics->second.first);
			if (iterStatistics != statistics.end())
				iterStatistics->second = iterCreateTimeStatistics->second.second;
		}
	}

	// tabulate character age statistics
	{
		std::map<int, std::pair<std::string, int> >::iterator iter = ageStatistics.begin();
		std::map<int, std::pair<std::string, int> >::iterator previousIter = ageStatistics.begin();

		for (; iter != ageStatistics.end(); ++iter)
		{
			if (iter != previousIter)
				iter->second.second += previousIter->second.second;

			previousIter = iter;
		}
	}

	// report character age statistics
	{
		for (std::map<int, std::pair<std::string, int> >::const_iterator iterAgeStatistics = ageStatistics.begin(); iterAgeStatistics != ageStatistics.end(); ++iterAgeStatistics)
		{
			std::map<std::string, int>::iterator iterStatistics = statistics.find(iterAgeStatistics->second.first);
			if (iterStatistics != statistics.end())
				iterStatistics->second = iterAgeStatistics->second.second;
		}
	}

	return statistics;
}

// ======================================================================

LfgCharacterSearchResultData::LfgCharacterSearchResultData() :
	characterId(),
	characterName(),
	species(SharedCreatureObjectTemplate::SP_human),
	profession(LfgCharacterData::Prof_Unknown),
	level(0),
	faction(0),
	guildName(),
	guildAbbrev(),
	groupId(),
	locationPlanet(),
	locationRegion(),
	locationPlayerCity(),
	ctsSourceGalaxy(),
	characterInterests()
{
}

// ----------------------------------------------------------------------

LfgCharacterSearchResultData::LfgCharacterSearchResultData(const LfgCharacterData & lfgData) :
	characterId(lfgData.characterId),
	characterName(lfgData.characterName),
	species(lfgData.species),
	profession(lfgData.profession),
	level(lfgData.level),
	faction(lfgData.faction),
	guildName(lfgData.guildName),
	guildAbbrev(lfgData.guildAbbrev),
	groupId(lfgData.groupId),
	locationPlanet(lfgData.locationPlanet),
	locationRegion(lfgData.locationRegion),
	locationPlayerCity(lfgData.locationPlayerCity),
	ctsSourceGalaxy(lfgData.ctsSourceGalaxy),
	characterInterests(lfgData.characterInterests)
{
}

// ======================================================================
