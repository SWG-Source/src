// ======================================================================
//
// ConsoleCommandParserObject.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ConsoleCommandParserObject.h"

#include "UnicodeUtils.h"
#include "serverGame/AiCreatureController.h"
#include "serverGame/BattlefieldMarkerObject.h"
#include "serverGame/BuildingObject.h"
#include "serverGame/Chat.h"
#include "serverGame/Client.h"
#include "serverGame/CommandQueue.h"
#include "serverGame/CommoditiesMarket.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ConsentManager.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/InstallationObject.h"
#include "serverGame/IntangibleObject.h"
#include "serverGame/MoveSimManager.h"
#include "serverGame/NameManager.h"
#include "serverGame/NewbieTutorial.h"
#include "serverGame/ObjectCommandParserGetAttributes.h"
#include "serverGame/ObjectCommandParserSetAttributes.h"
#include "serverGame/PlayerCreationManagerServer.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/ProsePackageManagerServer.h"
#include "serverGame/Region.h"
#include "serverGame/RegionMaster.h"
#include "serverGame/ServerController.h"
#include "serverGame/ServerManufactureSchematicObjectTemplate.h"
#include "serverGame/ServerMessageForwarding.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerShipObjectTemplate.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/ShipObject.h"
#include "serverGame/TangibleObject.h"
#include "serverGame/TriggerVolume.h"
#include "serverGame/UniverseObject.h"
#include "serverNetworkMessages/AccountFeatureIdRequest.h"
#include "serverNetworkMessages/AdjustAccountFeatureIdRequest.h"
#include "serverNetworkMessages/LocateStructureMessage.h"
#include "serverNetworkMessages/ReloadTemplateMessage.h"
#include "serverNetworkMessages/RenameCharacterMessage.h"
#include "serverNetworkMessages/TransferCharacterData.h"
#include "serverNetworkMessages/TransferCharacterDataArchive.h"
#include "serverNetworkMessages/UnloadObjectMessage.h"
#include "serverPathfinding/CityPathGraphManager.h"
#include "serverPathfinding/PathAutoGenerator.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "serverUtility/LocationData.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedCollision/Footprint.h"
#include "sharedFile/FileManifest.h"
#include "sharedFoundation/CalendarTime.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/Production.h"
#include "sharedGame/CommandTable.h"
#include "sharedGame/PlatformFeatureBits.h"
#include "sharedGame/PlayerData.h"
#include "sharedGame/ProsePackage.h"
#include "sharedGame/SharedStringIds.h"
#include "sharedLog/Log.h"
#include "sharedMath/Quaternion.h"
#include "sharedNetworkMessages/AIDebuggingMessages.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/MessageQueueDataTransform.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/ContainedByProperty.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/SlottedContainer.h"
#include "sharedObject/VolumeContainer.h"
#include "sharedObject/VolumeContainmentProperty.h"
#include "sharedSkillSystem/SkillObject.h"
#include "sharedTerrain/TerrainObject.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include <cstdio>

#if defined(_WIN32) && PRODUCTION == 0
#include "sharedDebug/VTune.h"
#endif

namespace ConsoleCommandParserObjectNamespace
{
	//test handler, this should eventually be removed
	void testConsentHandler(const NetworkId& player, int id, bool response)
	{
		int i = 0;
		if (response == true)
			i = 1;
		DEBUG_REPORT_LOG(true, ("We received a test consent back with values NetworkId:%s Id:%d Response:%d\n", player.getValueString().c_str(), id, i));
	}

	ServerObjectTemplate const *getObjectTemplateForCreation(std::string const &templateName)
	{
		// only allow non-ship ServerObjectTemplates
		ObjectTemplate const * const ot = ObjectTemplateList::fetch(templateName);
		if (ot)
		{
			ServerObjectTemplate const * const sot = ot->asServerObjectTemplate();

			if (!sot)
			{
				WARNING(true, ("ConsoleCommandParserObject invalid object template [%s]", templateName.c_str()));
				ot->releaseReference();
				return nullptr;
			}

			if (sot->getId() == ServerShipObjectTemplate::ServerShipObjectTemplate_tag)
			{
				SharedObjectTemplate const * const sharedTemplate = safe_cast<SharedObjectTemplate const *>(ObjectTemplateList::fetch(sot->getSharedTemplate()));
				if (nullptr == sharedTemplate || 
					(sharedTemplate->getGameObjectType() != SharedObjectTemplate::GOT_ship_mining_asteroid_dynamic &&
					sharedTemplate->getGameObjectType() != SharedObjectTemplate::GOT_ship_mining_asteroid_static))
				{
					ot->releaseReference();
					return nullptr;
				}
			}
			return sot;
		}
		return nullptr;
	}

	void checkBadBuildClusterObject(ServerObject *& o)
	{
		if (   ConfigServerGame::getBuildCluster()
		    && o
		    && !o->asTangibleObject()
		    && !o->asStaticObject())
		{
			o->permanentlyDestroy(DeleteReasons::SetupFailed);
			o = 0;
		}
	}

	typedef std::vector<std::string> StringVector;
	typedef std::vector<std::string> StoredStrings;
	typedef std::map<NetworkId , StoredStrings> PlayerStoredStringMap;
	PlayerStoredStringMap s_playerObjectTemplateMap;
	PlayerStoredStringMap s_playerCreatureNameMap;
	int const MAX_STORED_STRINGS = 50;

	std::string getStringShortcut(NetworkId const & userId, std::string const & tmpString, PlayerStoredStringMap const & playerStoredStringMap)
	{
		int stringIndex = 0;
		sscanf(tmpString.c_str(), "%d", &stringIndex);
		// decrement the string index so it starts at 0
		--stringIndex;

		// if no shortcut number was entered, return the original string
		if (stringIndex < 0)
			return tmpString;

		PlayerStoredStringMap::const_iterator objectMapIter = playerStoredStringMap.find(userId);

		if (objectMapIter != playerStoredStringMap.end())
		{
			const StoredStrings & storedStrings = objectMapIter->second;

			if (stringIndex >= 0  && stringIndex < (int)storedStrings.size())
			{
				return storedStrings[stringIndex];
			}
		}

		return tmpString;
	}

	std::string getObjectTemplateShortcut(NetworkId const & userId, std::string const & tmpString)
	{
		return getStringShortcut(userId, tmpString, s_playerObjectTemplateMap);
	}

	std::string getCreatureNameShortcut(NetworkId const & userId, std::string const & tmpString)
	{
		return getStringShortcut(userId, tmpString, s_playerCreatureNameMap);
	}

	char const * const s_creaureDataTable = "datatables/mob/creatures.iff";

	typedef std::string (*searchNamesFormatterFunction) (std::string const & preFormattedString, int const index);

	std::string searchNamesCreatureFormatter(std::string const & creatureName, int const index)
	{
		bool const openIfNotFound = true;
		DataTable const * const dataTable = DataTableManager::getTable(s_creaureDataTable, openIfNotFound);

		std::string result;

		if (dataTable)
		{
			int const row = dataTable->searchColumnString(0, creatureName);

			if (row > -1)
			{
				int const baseLevel = dataTable->getIntValue("BaseLevel", row);
				int const difficultyClass = dataTable->getIntValue("difficultyClass", row);
				std::string const socialGroup = dataTable->getStringValue("socialGroup", row);
				std::string const pvpFaction = dataTable->getStringValue("pvpFaction", row);
				bool const invulnerable = dataTable->getIntValue("invulnerable", row);
				float const aggressive = dataTable->getFloatValue("aggressive", row);
				float const assist = dataTable->getFloatValue("assist", row);
				int const deathBlow = dataTable->getIntValue("death_blow", row);

				FormattedString<512> fs;

				result += fs.sprintf("\t%4d - L:%3d (%-7s) %s (%s,%s",
					index, baseLevel,
					difficultyClass == 0 ? "NORMAL" : difficultyClass == 1 ? "ELITE" : difficultyClass == 2 ? "BOSS" : "UNKNOWN",
					creatureName.c_str(),
					socialGroup.empty() ? "NONE" : socialGroup.c_str(),
					pvpFaction.empty() ? "NONE" : pvpFaction.c_str());

				if (aggressive > 0)
					result += fs.sprintf(",AGGRO:%.0f", aggressive);

				if (assist > 0)
					result += fs.sprintf(",ASSIST:%.0f", assist);

				if (deathBlow > 0)
					result += ",DB";

				if (invulnerable)
					result += ",INV";

				result += ")\n";
			}
		}

		return result.empty() ? FormattedString<512>().sprintf("\t%4d - %s\n", index, creatureName.c_str()) : result;
	}

	std::string searchNamesDefaultFormatter(std::string const & preFormattedString, int const index)
	{
		return FormattedString<512>().sprintf("\t%4d - %s\n", index, preFormattedString.c_str());
	}

	//-- given a vector of string names
	std::string searchNames(StringVector const & namesToSearch, StringVector const & desiredSubStrings,
		char const * const exclusionString, StringVector & foundNames, searchNamesFormatterFunction formatterFunction = searchNamesDefaultFormatter)
	{
		std::string result;
		bool tooManyEntries = false;

		StringVector::const_iterator itName = namesToSearch.begin();
		for (; itName != namesToSearch.end(); ++itName)
		{
			if (!itName->empty())
			{
				std::string const name(*itName);

				// if it matches the exclusion string, skip
				if (exclusionString && name.find(exclusionString) != std::string::npos)
					continue;

				// check all desired substrings
				bool skipName = false;
				for (int i = 0; i < (int)desiredSubStrings.size(); ++i)
				{
					if (name.find(desiredSubStrings[i]) == std::string::npos)
					{
						skipName = true;
						break;
					}
				}

				if (skipName)
					continue;

				// check for too many entries
				if (foundNames.size() >= (int)MAX_STORED_STRINGS)
				{
					tooManyEntries = true;
					break;
				}

				// otherwise, we have one we want, add it to the vector
				foundNames.push_back(name);
			}
		}

		if (foundNames.size() == 0)
		{
			result += "Unable to find any matching objects.\n";
		}
		else
		{
			std::sort(foundNames.begin(), foundNames.end());
			for (int k = 0; k < (int)foundNames.size(); ++k)
			{
				result += formatterFunction ? formatterFunction(foundNames[k], k+1) : searchNamesDefaultFormatter(foundNames[k], k+1);
			}

			if (tooManyEntries)
				result += "\tToo many entries!  Please refine search.\n";
		}

		return result;
	}

#ifdef USE_ALTERNATE_BUILD_REGION_CALCULATION
	bool isInNoBuildRegion( real x, real y )
	{
		DEBUG_REPORT_LOG_PRINT( true, ( "isInNoBuildRegion: %f, %f\n", x, y ) );

		RegionMaster::RegionVector v;
		RegionMaster::getRegionsAtPoint( ConfigServerGame::getSceneID(), x, y, v );

		RegionMaster::RegionVector::const_iterator it;

		for ( it = v.begin(); it != v.end(); ++it )
		{
			DEBUG_REPORT_LOG_PRINT( true, ( "checking region '%s' %d\n", Unicode::wideToNarrow( (*it)->getName() ).c_str(), (*it)->getBuildable() ) );
			if ( (*it)->getBuildable() == 0 )
			{
				return true;
			}
		}

		return false;
	}
#endif

	std::set<std::string> s_unmoveable;
}

using namespace ConsoleCommandParserObjectNamespace;

// ======================================================================

static const CommandParser::CmdInfo cmds[] =
{
	{"addLocationTarget",               6, "<oid> <name> <x> <y> <z> <radius>",  "Sets a location target for an object. onArrivedAtLocation will be triggered on the object, and messageHandler OnArrivedAtLocation() invoked on attached scripts."},
	{"authority",                       2, "<oid> <gameserver process id>",      "Transfer authority of object <oid> to gameserver <id>."},
	{"create",                          4, "<templatename> <x> <y> <z>",         "Create a new object."},
	{"cellCreate",                      4, "<templatename> <cell> <x> <y> <z>",  "Create a new object in a cell."},
	{"count",                           0, "",                           "Returns the number of objects allocated on this game server"},
	{"createAt",                        1, "<templatename>",             "Create a new object where you are standing."},
	{"createManyAt",                    2, "<count> <templatename>",     "Create new objects around where you are standing."},
	{"createIn",                        2, "<templatename> <oid>",       "Create a new object in a container."},
	{"createTranslateRotate",           8, "<templatename> <x> <y> <z> <quaternionW> <quaternionX> <quaternionY> <quaternionZ>", "Create a new object."},
	{"cellCreateTranslateRotate",       8, "<templatename> <cell> <x> <y> <z> <quaternionW> <quaternionX> <quaternionY> <quaternionZ>", "Create a new object in a cell."},
	{"createTriggerVolume",             3, "<oid> <name> <radius>",      "Create a new trigger volume on the specified object."},
	{"destroy",                         1, "<oid>",                      "Destroy an existing object."},
	{"destroyAllMobsExcept",            1, "<oid>",                      "Destroy all mobs except the given one."},
	{"destroyAllWorldMobs",             0, "",                           "Destroy all mobs in the world cell."},
	{"find",                            1, "<oid>",                      "Find the coords of an existing object."},
	{"findCreature",                    1, "<string1> [string2] ...",    "Search for creature names containing all string arguments. Index may be used instead of string name for commands that use a creature name (like spawn). Output displays:\n\tindex - L:level (difficulty class) name (social group, pvp faction, [aggro radius, assist radius, Death Blow, INVulernable]) where square bracketed [] fields display only if applicable."},
	{"findTemplate",                    1, "<templatename>",             "Find ."},
	{"findRelative_w",                  1, "<oid>",                      "Find the your coords relative to an existing object."},
	{"fixHouseItemLimit",               1, "<oid>",                      "Fixup item limit."},
	{"getHeight",                       0, "",                           "Get the height where the user is standing."},
	{"getInventoryId",                  1, "<oid>",                      "Get the inventory object for the specified creature."},
	{"getDatapadId",                    1, "<oid>",                      "Get the datapad object for the specified player."},
	{"getServerTemplate",               1, "<oid>",                      "Get the server template name of an object."},
	{"getExposedProxyList",             1, "<oid>",                      "Get the exposed list of servers the specified object is proxied on."},
	{"getPrivateProxyList",             1, "<oid>",                      "Get the private list of servers the specified object is proxied on."},
	{"getObservers",                    1, "<oid>",                      "Get the list of clients currently observing the specified object on this game server."},
	{"getObserving",                    1, "<oid>",                      "Get the list of objects this client is observing. (brief version)"},
	{"getObservingDetailed",            1, "<oid>",                      "Get the list of objects this client is observing. (detailed version)"},
	{"getObservingPvpSync",             1, "<oid>",                      "Get the list of pvp sync objects this client is observing. (brief version)"},
	{"getObservingPvpSyncDetailed",     1, "<oid>",                      "Get the list of pvp sync objects this client is observing. (detailed version)"},
	{"getLoadWith",                     1, "<oid>",                      "Get the loadWith container of an object.h"},
	{"getVolume",                       1, "<oid>",                      "Get the volume of an object"},
	{"gotoObject",                      1, "<oid>",                      "Goto the coords of an existing object. currently disabled."},
	{"isIncapacitated",                 1, "<oid>",                      "Gets the incapacitated state of a creature"},
	{"isProxy",                         1, "<oid>",                      "Query as to whether this is a proxy on the avatar's server."},
	{"listContainer",                   1, "<oid>",                      "Lists the contents of a volume container object."},
	{"loadContents",                    1, "<container> [oid]",          "Load an object in the specified demand-load container.  If oid is not specified, the entire contents are loaded."},
	{"move",                            4, "<oid> <x> <y> <z>",          "Move an existing object."},
	{"moveToMe",                        1, "<oid>",                      "Move an existing object to my coordinates."},
	{"persist",                         1, "<oid>",                      "Persist an object."},
	{"reloadServerTemplate",            1, "<templatename>",             "Reloads the given server object template."},
	{"removeLocationTarget",            2, "<oid> <name>",               "Removes a location target from an object."},
	{"removeTriggerVolume",             2, "<oid> <name>",               "Remove the named trigger volume from the specified object."},
	{"renamePlayer",                    2, "<oid> <name>",               "Rename a player.  Naming rules and uniqueness are not enforced."},
	{"rotate",                          4, "<oid> <yaw> <pitch> <roll>", "Rotate an existing object."},
	{"scaleMovement",                   2, "<oid> <scale>",              "Set the movement scale on a creature."},
	{"scaleSize",                       2, "<oid> <scale>",              "Set the size scale of a creature."},
	{"snap",                            1, "<oid>",                      "Snap an object to the terrain."},
	{"spawn",                           1, "<creature/poi name> [number] [distance] [spacing]", "Create [number](1) creature objects at [distance](1.25) meters with [spacing](creature's collision diameter) meters from the player's position and heading."},
	{"stopWatching",                    1, "<oid>",                      "Stops watching an object."},
	{"triggerVolumes",                  1, "<oid>",                      "List trigger volumes on an object"},
	{"setName"    ,                     1, "<oid> <name>",               "Set name."},
	{"watch",                           1, "<oid>",                      "Watches an object. Prevents it from unloading on the server (it will still be removed from the client) when there are no players in range."},
	{"watchAIPath",                     2, "<oid> <bool>",               "Enable or disable sending of the selected object's path"},
	{"watchPathMap",                    1, "<bool>",                     "Enable or disable sending the path map"},
	{"unstick",                         0, "",                           "Ask the server to get this object un-stuck if it's become stuck due to collision problems"},
	{"attribs",                         1, "<oid>",                      "Prints the attributes of a creature."},
	{"enqueueCommand",                  4, "<actor_oid> <commandName> <target_oid> <params>",    "enqueue a command in the actor's command queue"},
	{"hide",                            2, "<oid> <bool>",               "Set the target as invisible (1) or visible(0)"},
	{"setCoverVisibility",              2, "<oid> <bool>",               "Set the target cover visibility as visible (1) or invisible(0)"},
	{"getAttributes",                   1, "<oid>",                      "Get the list of attributes that would be sent in a AttributesMessage for the specified object"},
	{"createPathNodes",                 1, "<oid>",                      "Create city path nodes for the given object"},
	{"destroyPathNodes",                1, "<oid>",                      "Destroy city path nodes for the given object"},
	{"reloadPathNodes",                 0, "",                           "Reload all city path nodes from the database"},
	{"relinkPathGraph",                 0, "",                           "Relink all path nodes in the current city"},
	{"setPathLinkDistance",             1, "<distance>",                 "Set the maximum distance that path nodes will link"},
	{"markCityEntrance",                1, "<oid>",                      "Mark the given object as a city entrance node"},
	{"unmarkCityEntrance",              1, "<oid>",                      "Remove the city entrance mark from the given object"},
	{"setOwner",                        1, "<oid> [owner]",              "Sets the owner of an object"},
	{"getOwner",                        1, "<oid>",                      "gets the owner of an object"},
	{"askConsent",                      0, "",                           "TEST: ask a test consent question"},
	{"debugFar",                        2, "<oid> <bool> <optional radius>",  "Create or destroy a far volume on an object. If a radius is specified, the object will use that radius."},
	{"addJediSlot",                     1, "<oid>",                      "Test a character earning a Jedi slot for his player (oid is the objectId of the player's creature object)"},
	{"getJediSlotInfo",                 1, "<oid>",                      "Get various information about the Jedi slot for the character"},
	{"makeSpectral",                    1, "<oid>",                      "Test making a character spectral in the account database."},
	{"getHibernating",                  1, "<oid>",                      "Get the hibernate flag for a creature"},
	{"getBehavior",                     1, "<oid>",                      "Get the behavior type for a creature"},
	{"getStandingOn",                   1, "<oid>",                      "Get the network ID of the object a creature is standing on"},
	{"getPower",                        1, "<oid>",                      "Get the amount of power in an installation"},
	{"setPower",                        2, "<oid> power",                "Set the amount of power in an installation"},
	{"getPowerRate",                    1, "<oid>",                      "Get the power consuption (units/hr) of an installation"},
	{"setPowerRate",                    2, "<oid> rate",                 "Set the power consuption (units/hr) of an installation"},
	{"getBattlefieldMarkerRegionName",  1, "<oid>",                      "Get the region name of a battlefield marker"},
	{"setBattlefieldMarkerRegionName",  2, "<oid> <region name>",        "Set the region name of a battlefield marker"},
	{"unload",                          1, "<oid>",                      "Unload an object on all servers"},
	{"moveToPlayerAndUnload",           2, "<oid> <target player>",      "Move an object to the specified offline player and unload the object"},
	{"resetStats",                      1, "<oid>",                      "Resets the stats of the given player to their starting values"},
	{"restoreHouse",                    1, "<house oid>",                "Restore a house in the database.  (Do not use this to restore any other type of object.)"},
	{"restoreCharacter",                1, "<character oid>",            "Restore a character in the database.  (Do not use this to restore any other type of object.)"},
	{"enableCharacter",                 2, "<StationId> <PlayerId>",     "enable a character to play"},
	{"disableCharacter",                2, "<StationId> <PlayerId>",     "disable a character to play"},
	{"loginCharacter",                  1, "<PlayerId>",                 "logs in a character"},
	{"undeleteItem",                    1, "<item oid>",		         "Undelete an item including house or character."},
	{"loadStructure",                   1, "<item oid>",                 "Load a structure into game.  This structure must not exist in game."},
	{"deleteAuctionLocation",           1, "<LocationId>",               "Delete an auction location from commodities server and database.  The LocationId is either vendorId for droid vendors, or vendorInventoryId for NPC vendors"},
	{"moveToPlayer",                    2, "<oid> <playerOID>",          "Move an object to a player"},
	{"moveSimStart",                    4, "<countNPC> <countPC> <radius> <speed>",                           ""},
	{"moveSimClear",                    0, "",                           ""},
	{"deletePersistedContents",         1, "<building oid>",             "Delete all the persisted contents in a house. (useful for objects in walls etc)"},
	{"moveItemInHouseToMe",             2, "<building oid> <index>",     "Moves the first item in a house to me. An index of 0 indicates first item"},
	{"reinitializeVendor",              1, "<vendor oid>",               "Reinitialize a vendor moving all items to the stockroom"},
	{"testVendorStatus",                2, "<vendor oid> <status>",      "Status test"},
	{"setAdminTitle",                   1, "<0=none, 1=csr, 2=dev, 3=qa>",     "Set your development title 0=none, 1=csr, 2=dev, 3=qa"},
	{"playUiEffect",                    0, "<player oid> <effect string>", "Play a UI effect on the target client."},
	{"rollupStructure",                 2, "<house oid> <owner oid>",    "Rollup a house and transfer the contents to the specified player's bank box."},
	{"regionList",                      0, "",                           "List regions at point."},
	{"canMove",                         0, "",                           "Can move to target"},
	{"pathAutoGenerate",                2, "<node dist> <obstacle dist>", "gen"},
	{"pathAutoCleanup",                 0, "",                           "clean"},
	{"getStaticInfo",                   1, "<oid>",                      "Get the Static Item Information of an object"},
	{"getBroadcastInfo",                1, "<oid>",                      "Get the broadcasting information of an object"},
	{"getAge",                          1, "<oid>",                      "Get the age (in days) for the specified character"},
	{"adjustAge",                       2, "<oid> <adjustment>",         "Adjust the age (in days) for the specified character"},
	{"getBadge",                        1, "<oid>",                      "Get the earned and unearned badges for the specified character"},
	{"getPassiveReveal",                1, "<stealth/hidden object oid>", "Get the passive reveal list for the specified stealth/hidden object"},
	{"addPassiveReveal",                3, "<stealth/hidden object oid> <target oid> <range>", "Add the target object to the passive reveal list for the specified stealth/hidden object"},
	{"removePassiveReveal",             2, "<stealth/hidden object oid> <target oid>", "Remove the target object from the passive reveal list for the specified stealth/hidden object"},
	{"clearPassiveReveal",              1, "<stealth/hidden object oid>", "Clear the passive reveal list for the specified stealth/hidden object"},
	{"getInvulnerabilityTimer",         1, "<oid>", "Get the invulnerability timer for the specified creature object"},
	{"spewCommandQueue",                1, "<oid>", "Dumps the command queue for the specified creature object"},
	{"fixLoadWith",                     3, "<topmost_object_oid> <starting_loadwith_oid> <max_depth>", "Fix the load_with for topmost_object and its contents by setting them to starting_loadwith using max_depth"},
	{"getChatStatistics",               1, "<oid>", "Get the chat statistics for the specified character"},
	{"getOpenedContainers",             1, "<oid>", "Get the set of opened containers for the specified character"},
	{"adjustSwgAccountFeatureId",       3, "<oid> <feature Id number> <adjustment>", "Adjust the specified SWG feature Id number of the account of the specified character by the specified amount"},
	{"reloadSwgAccountFeatureId",       1, "<oid>", "Retrieve the SWG account feature Ids for the specified character"},
	{"reloadSwgTcgAccountFeatureId",    1, "<oid>", "Retrieve the SWG TCG account feature Ids for the specified character"},
	{"unloadBuildingContents",          1, "<oid>", "Unload the contents of a player-placed authoritative demand-loaded building"},
	{"isOnSolidFloor",                  1, "<oid>", "Checks to see if the specified object is \"on solid floor\""},

	{"", 0, "", ""} // this must be last
};

//-----------------------------------------------------------------

ConsoleCommandParserObject::ConsoleCommandParserObject (void) :
CommandParser ("object", 0, "...", "Object related commands.", 0),
Emitter()
{
	createDelegateCommands (cmds);
	addSubCommand(new ObjectCommandParserGetAttributes());
	addSubCommand(new ObjectCommandParserSetAttributes());
}

//-----------------------------------------------------------------


bool ConsoleCommandParserObject::performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
	CreatureObject * const playerObject = dynamic_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(userId));
	if (!playerObject)
	{
		WARNING_STRICT_FATAL(true, ("Console command executed on invalid player object %s", userId.getValueString().c_str()));
		return false;
	}
		
	UNREF(originalCommand);

	if (s_unmoveable.empty())
	{
		// if you add an item type here, also add it to move_item_to_player() in admin.plsql
		IGNORE_RETURN(s_unmoveable.insert("object/tangible/inventory/character_inventory.iff"));
		IGNORE_RETURN(s_unmoveable.insert("object/tangible/mission_bag/mission_bag.iff"));
		IGNORE_RETURN(s_unmoveable.insert("object/tangible/datapad/character_datapad.iff"));
		IGNORE_RETURN(s_unmoveable.insert("object/tangible/bank/character_bank.iff"));
		IGNORE_RETURN(s_unmoveable.insert("object/weapon/melee/unarmed/unarmed_default_player.iff"));
		IGNORE_RETURN(s_unmoveable.insert("object/player/player.iff"));
		IGNORE_RETURN(s_unmoveable.insert("object/cell/cell.iff"));
		IGNORE_RETURN(s_unmoveable.insert("object/tangible/inventory/vendor_inventory.iff"));
	}

	//-----------------------------------------------------------------

	if (isCommand (argv [0], "getOpenedContainers"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject const * const obj = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(oid));
		if (obj == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		Client const * const client = obj->getClient();

		if(client == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a client object\n");
			return true;
		}

		std::set<ServerObject*> const& opened = client->getOpenedContainers();

		Unicode::String text;

		for(std::set<ServerObject*>::const_iterator i = opened.begin(); i != opened.end(); ++i)
		{
			ServerObject const * const openedobj = *i;

			if(openedobj)
			{
				text += Unicode::narrowToWide(openedobj->getNetworkId().getValueString());

				const char *objectTemplateName = openedobj->getObjectTemplateName();
				if (objectTemplateName)
				{
					text += Unicode::narrowToWide(std::string(", "));
					text += Unicode::narrowToWide(std::string(objectTemplateName));
				}

				text += Unicode::narrowToWide(std::string("\n"));
			}
		}

		const char *objectTemplateName = obj->getObjectTemplateName();
		result += Unicode::narrowToWide(FormattedString<512>().sprintf("object %s (%s) has %d total opened objects\n", obj->getNetworkId().getValueString().c_str(), (objectTemplateName ? objectTemplateName : "nullptr object template"), opened.size()));

		if(!text.empty())
		{
			result += text;
		}

		return true;
	}

	//-----------------------------------------------------------------
	if (isCommand( argv [0], "spawn"))
	{
		const int optionalParameterIndex = 2;
		const uint argvSize = argv.size();

		Unicode::String tmpString = Unicode::narrowToWide("create ");

		// append the creature name which may be a shortcut
		std::string creatureName = getCreatureNameShortcut(userId, Unicode::wideToNarrow(argv[1]));
		tmpString.append(Unicode::narrowToWide(creatureName));

		// concatenate the optional arguments
		for (uint i=optionalParameterIndex; i < argvSize; ++i)
		{
			tmpString.append(Unicode::narrowToWide(" ").append(argv[i]));
		}
		
		ServerObject * userObject = ServerWorld::findObjectByNetworkId(userId);
		if (!userObject)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_USER);
			return true;
		}

		GameScriptObject* gso = userObject->getScriptObject();
		if (gso)
		{
			gso->attachScript("beta.test_create", true);
			ScriptParams params;
			params.addParam(tmpString);
			IGNORE_RETURN (gso->trigAllScripts(Scripting::TRIG_SPEAKING, params) != SCRIPT_CONTINUE);
			gso->detachScript("beta.test_create");
		}
		
		result += getErrorMessage (argv[0], ERR_SUCCESS);
	}

	else if (isCommand(argv[0], "authority"))
	{
		NetworkId const oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * const object = dynamic_cast<ServerObject *>(NetworkIdManager::getObjectById(oid));
		if (object)
		{
			uint32 const pid = (strtoul(Unicode::wideToNarrow(argv[2]).c_str (), nullptr, 10));
			GenericValueTypeMessage<std::pair<NetworkId, uint32> > const msg(
				"RequestAuthTransfer",
				std::make_pair(
					ContainerInterface::getTopmostContainer(*object)->getNetworkId(),
					pid));
			GameServer::getInstance().sendToPlanetServer(msg);

			result += getErrorMessage (argv[0], ERR_SUCCESS);
		}
		else
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
		}
		
	}

	else if (isCommand( argv [0], "create"))
	{
		//Should be server object
		std::string tmpString(getObjectTemplateShortcut(userId, Unicode::wideToNarrow(argv[1])));

		//Should be Server object
		ServerObject * userObject = ServerWorld::findObjectByNetworkId(userId);
		if (!userObject)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_USER);
			return true;
		}

		real x,y,z;
		x = static_cast<real>(strtod(Unicode::wideToNarrow(argv[2]).c_str(), nullptr));
		y = static_cast<real>(strtod(Unicode::wideToNarrow(argv[3]).c_str(), nullptr));
		z = static_cast<real>(strtod(Unicode::wideToNarrow(argv[4]).c_str(), nullptr));

		ServerObject *newObject = 0;
		ServerObjectTemplate const * const ot = getObjectTemplateForCreation(tmpString);
		if (ot)
		{
			if (ConfigServerGame::getStripNonFreeAssetsFromPlayersInTutorial() && NewbieTutorial::isInTutorialArea(userObject))
			{
				std::string sharedTemplate = ot->getSharedTemplate();

				if (!FileManifest::contains(sharedTemplate.c_str()))
				{
					result += getErrorMessage (argv[0], ERR_NOT_SKUFREE_TEMPLATE);
					ot->releaseReference();
					return true;
				}
			}
			
			Transform tr;
			tr.setPosition_p(x, y, z);
			newObject = ServerWorld::createNewObject(*ot, tr, 0, false);
			checkBadBuildClusterObject(newObject);
			ot->releaseReference();
		}

		if (!newObject)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_TEMPLATE);
			return true;
		}
		else
		{
			newObject->addToWorld();
		}

		if (ConfigServerGame::getAdminPersistAllCreates())
			newObject->persist();

		result += Unicode::narrowToWide("NetworkId: ");
		result += Unicode::narrowToWide(newObject->getNetworkId().getValueString());
		result += Unicode::narrowToWide("\n");
		result += getErrorMessage (argv[0], ERR_SUCCESS);
	}

	else if (isCommand( argv [0], "createTranslateRotate"))
	{
		//Should be server object
		std::string tmpString(getObjectTemplateShortcut(userId, Unicode::wideToNarrow(argv[1])));

		//Should be Server object
		ServerObject * userObject = ServerWorld::findObjectByNetworkId(userId);
		if (!userObject)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_USER);
			return true;
		}

		real x,y,z;
		x = static_cast<real>(strtod(Unicode::wideToNarrow(argv[2]).c_str(), nullptr));
		y = static_cast<real>(strtod(Unicode::wideToNarrow(argv[3]).c_str(), nullptr));
		z = static_cast<real>(strtod(Unicode::wideToNarrow(argv[4]).c_str(), nullptr));

		// check to see if we're in a region we shouldn't be building in
		if ( ConfigServerGame::getBlockBuildRegionPlacement() )
		{
			const Region * const r = RegionMaster::getSmallestRegionAtPoint( ConfigServerGame::getSceneID(), x, z );

			if ( r && r->getBuildable() )
			{
				DEBUG_REPORT_LOG_PRINT( true, ( "unable to place object in %s\n", Unicode::wideToNarrow( r->getName() ).c_str() ) );
				result += Unicode::narrowToWide("Unable to place object: the region '");
				result += r->getName();
				result += Unicode::narrowToWide("' is marked Buildable.\n");
				return true;
			}
		}
 
		real quatW,quatX,quatY,quatZ;
		quatW = static_cast<real>(strtod(Unicode::wideToNarrow(argv[5]).c_str(), nullptr));
		quatX = static_cast<real>(strtod(Unicode::wideToNarrow(argv[6]).c_str(), nullptr));
		quatY = static_cast<real>(strtod(Unicode::wideToNarrow(argv[7]).c_str(), nullptr));
		quatZ = static_cast<real>(strtod(Unicode::wideToNarrow(argv[8]).c_str(), nullptr));

		ServerObject * const cell = safe_cast<ServerObject *>(ContainerInterface::getContainingCellObject(*playerObject));

		ServerObject *newObject = 0;
		ServerObjectTemplate const * const ot = getObjectTemplateForCreation(tmpString);
		if (ot)
		{
			if (ConfigServerGame::getStripNonFreeAssetsFromPlayersInTutorial() && NewbieTutorial::isInTutorialArea(userObject))
			{
				std::string sharedTemplate = ot->getSharedTemplate();

				if (!FileManifest::contains(sharedTemplate.c_str()))
				{
					result += getErrorMessage (argv[0], ERR_NOT_SKUFREE_TEMPLATE);
					ot->releaseReference();
					return true;
				}
			}

			Quaternion q(quatW, quatX, quatY, quatZ);
			Transform tr(Transform::IF_none);
			//get quaternion rotation as a transform
			q.getTransform(&tr);
			// set the position
			tr.setPosition_p(x, y, z);
			newObject = ServerWorld::createNewObject(*ot, tr, cell, false);
			checkBadBuildClusterObject(newObject);
			ot->releaseReference();
		}

		if (!newObject)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_TEMPLATE);
			return true;
		}
		else if (!cell)
			{
				newObject->addToWorld();
			}

		if (ConfigServerGame::getAdminPersistAllCreates())
		{
			newObject->persist();
		}
		else
		{
			newObject->setIncludeInBuildout( true );
		}

		result += Unicode::narrowToWide("NetworkId: ");
		result += Unicode::narrowToWide(newObject->getNetworkId().getValueString());
		result += Unicode::narrowToWide("\n");
		result += getErrorMessage (argv[0], ERR_SUCCESS);
	}

	else if (isCommand( argv [0], "createAt"))
	{
		//Should be Server object
		ServerObject * userObject = ServerWorld::findObjectByNetworkId(userId);
		if (!userObject)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_USER);
			return true;
		}
		
		// Look for a matching template in the template map first.
		std::string tmpString(getObjectTemplateShortcut(userId, Unicode::wideToNarrow(argv[1])));

		ServerObjectTemplate const * const ot = getObjectTemplateForCreation(tmpString);
		if (!ot || ot->getId() == ServerManufactureSchematicObjectTemplate::ServerManufactureSchematicObjectTemplate_tag)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_TEMPLATE);
			if (ot)
				ot->releaseReference();
			return true;
		}

		if (ConfigServerGame::getStripNonFreeAssetsFromPlayersInTutorial() && NewbieTutorial::isInTutorialArea(userObject))
		{
			std::string sharedTemplate = ot->getSharedTemplate();

			if (!FileManifest::contains(sharedTemplate.c_str()))
			{
				result += getErrorMessage (argv[0], ERR_NOT_SKUFREE_TEMPLATE);
				ot->releaseReference();
				return true;
			}
		}

		//if the user is a creature and they're piloting a ship, use the ship's location
		CreatureObject * const creatureUserObject = userObject->asCreatureObject();
		if(creatureUserObject)
		{
			ShipObject * const shipUserObject = creatureUserObject->getPilotedShip();
			if(shipUserObject)
				userObject = shipUserObject;
		}

		ServerObject * const cell = safe_cast<ServerObject *>(ContainerInterface::getContainingCellObject(*userObject));

		Transform tr;
		tr.setPosition_p(userObject->getPosition_p());
		ServerObject *newObject = ServerWorld::createNewObject(*ot, tr, cell, false);
		checkBadBuildClusterObject(newObject);
		ot->releaseReference();

		if (!newObject)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_TEMPLATE);
			return true;
		}
		else if (!cell)
			{
				newObject->addToWorld();
			}

		if (ConfigServerGame::getAdminPersistAllCreates())
			newObject->persist();

		result += Unicode::narrowToWide("NetworkId: ");
		result += Unicode::narrowToWide(newObject->getNetworkId().getValueString());
		result += Unicode::narrowToWide("\n");
		result += getErrorMessage (argv[0], ERR_SUCCESS);
	}

	//-----------------------------------------------------------------

	else if (isCommand( argv [0], "createManyAt"))
	{
		// should be Server object
		ServerObject * const userObject = ServerWorld::findObjectByNetworkId(userId);
		if (!userObject)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_USER);
			return true;
		}

		if (ContainerInterface::getContainedByObject(*userObject))
		{
			result += getErrorMessage (argv[0], ERR_FAIL);
			return true;
		}

		Vector const      position  = userObject->getPosition_p();
		int const         count     = atoi(Unicode::wideToNarrow (argv [1]).c_str());
		std::string const tmpString = getObjectTemplateShortcut(userId, Unicode::wideToNarrow(argv[2]));

#if defined(_WIN32) && PRODUCTION == 0
		VTune::resume();
#endif

		ServerObjectTemplate const * const ot = getObjectTemplateForCreation(tmpString);
		if (ot)
		{
			if (ConfigServerGame::getStripNonFreeAssetsFromPlayersInTutorial() && NewbieTutorial::isInTutorialArea(userObject))
			{
				std::string sharedTemplate = ot->getSharedTemplate();

				if (!FileManifest::contains(sharedTemplate.c_str()))
				{
					result += getErrorMessage (argv[0], ERR_NOT_SKUFREE_TEMPLATE);
					ot->releaseReference();
					return true;
				}
			}
			
			for (int i = 0; i < count; ++i)
			{
				Vector const offset(static_cast<float>(i / 10), 0.0f, static_cast<float>(i % 10));
				Transform tr;
				tr.setPosition_p(position+offset);

				ServerObject * newObject = ServerWorld::createNewObject(*ot, tr, 0, false);
				checkBadBuildClusterObject(newObject);

				if (newObject)
				{
					newObject->addToWorld();
				}
			}
			ot->releaseReference();
		}

#if defined(_WIN32) && PRODUCTION == 0
		VTune::pause();
#endif

		result += getErrorMessage (argv[0], ERR_SUCCESS);
		return true;
	}

	//-----------------------------------------------------------------
	else if (isCommand(argv[0], "createIn"))
	{
		//Create a new object in a container.
		ServerObject * userObject = ServerWorld::findObjectByNetworkId(userId);
		if (!userObject)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_USER);
			return true;
		}

		NetworkId oid(Unicode::wideToNarrow(argv[2]));
		ServerObject* object = dynamic_cast<ServerObject*>(NetworkIdManager::getObjectById(oid));
		if (object == nullptr)
		{
			//container does not exist
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		VolumeContainer * volContainer = ContainerInterface::getVolumeContainer(*object);
		int oldCapacity = 0;
		if (volContainer)
			oldCapacity = volContainer->debugDoNotUseSetCapacity(-1);

		std::string tmpString(getObjectTemplateShortcut(userId, Unicode::wideToNarrow(argv[1])));

		ServerObject *newObject = 0;
		ServerObjectTemplate const * const ot = getObjectTemplateForCreation(tmpString);
		if (ot)
		{
			if (ConfigServerGame::getStripNonFreeAssetsFromPlayersInTutorial() && NewbieTutorial::isInTutorialArea(userObject))
			{
				std::string sharedTemplate = ot->getSharedTemplate();

				if (!FileManifest::contains(sharedTemplate.c_str()))
				{
					result += getErrorMessage (argv[0], ERR_NOT_SKUFREE_TEMPLATE);
					ot->releaseReference();
					return true;
				}
			}

			newObject = ServerWorld::createNewObject(*ot, *object, false, true);
			checkBadBuildClusterObject(newObject);

			ot->releaseReference();
		}

		if (volContainer)
		{
			volContainer->debugDoNotUseSetCapacity(oldCapacity);
			volContainer->recalculateVolume();
		}

		if (!newObject)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_TEMPLATE);
			return true;
		}

		if (ConfigServerGame::getAdminPersistAllCreates())
			newObject->persist();

		result += Unicode::narrowToWide("NetworkId: ");
		result += Unicode::narrowToWide(newObject->getNetworkId().getValueString());
		result += Unicode::narrowToWide("\n");
	  
		result += getErrorMessage (argv[0], ERR_SUCCESS);		
	}
	else if (isCommand(argv[0], "cellCreate"))
	{
		NetworkId cellId(Unicode::wideToNarrow(argv[2]));
		ServerObject* cell = dynamic_cast<ServerObject*>(NetworkIdManager::getObjectById(cellId));
		if (cell == nullptr && cellId != NetworkId::cms_invalid)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		std::string tmpString(Unicode::wideToNarrow(argv[1]));

		//Should be Server object
		ServerObject * userObject = ServerWorld::findObjectByNetworkId(userId);
		if (!userObject)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_USER);
			return true;
		}

		real x,y,z;
		x = static_cast<real>(strtod(Unicode::wideToNarrow(argv[3]).c_str(), nullptr));
		y = static_cast<real>(strtod(Unicode::wideToNarrow(argv[4]).c_str(), nullptr));
		z = static_cast<real>(strtod(Unicode::wideToNarrow(argv[5]).c_str(), nullptr));

		ServerObject *newObject = 0;
		ServerObjectTemplate const * const ot = getObjectTemplateForCreation(tmpString);
		if (ot)
		{
			if (ConfigServerGame::getStripNonFreeAssetsFromPlayersInTutorial() && NewbieTutorial::isInTutorialArea(userObject))
			{
				std::string sharedTemplate = ot->getSharedTemplate();

				if (!FileManifest::contains(sharedTemplate.c_str()))
				{
					result += getErrorMessage (argv[0], ERR_NOT_SKUFREE_TEMPLATE);
					ot->releaseReference();
					return true;
				}
			}
			
			Transform tr;
			tr.setPosition_p(x, y, z);
			newObject = ServerWorld::createNewObject(*ot, tr, cell, false);
			checkBadBuildClusterObject(newObject);
			ot->releaseReference();
		}
		
		if (!newObject)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_TEMPLATE);
			return true;
		}
		if (cellId == NetworkId::cms_invalid)
			newObject->addToWorld();

		if (ConfigServerGame::getAdminPersistAllCreates())
			newObject->persist();

		result += Unicode::narrowToWide("NetworkId: ");
		result += Unicode::narrowToWide(newObject->getNetworkId().getValueString());
		result += Unicode::narrowToWide("\n");
		result += getErrorMessage (argv[0], ERR_SUCCESS);
		
	}
	else if (isCommand(argv[0], "cellCreateTranslateRotate"))
	{
		//Should be server object
		std::string tmpString(Unicode::wideToNarrow(argv[1]));

		//Should be Server object
		ServerObject * userObject = ServerWorld::findObjectByNetworkId(userId);
		if (!userObject)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_USER);
			return true;
		}

		NetworkId cellId(Unicode::wideToNarrow(argv[2]));
		ServerObject * const cell = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(cellId));
		if (cell == nullptr && cellId != NetworkId::cms_invalid)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		real x,y,z;
		x = static_cast<real>(strtod(Unicode::wideToNarrow(argv[3]).c_str(), nullptr));
		y = static_cast<real>(strtod(Unicode::wideToNarrow(argv[4]).c_str(), nullptr));
		z = static_cast<real>(strtod(Unicode::wideToNarrow(argv[5]).c_str(), nullptr));

		real quatW,quatX,quatY,quatZ;
		quatW = static_cast<real>(strtod(Unicode::wideToNarrow(argv[6]).c_str(), nullptr));
		quatX = static_cast<real>(strtod(Unicode::wideToNarrow(argv[7]).c_str(), nullptr));
		quatY = static_cast<real>(strtod(Unicode::wideToNarrow(argv[8]).c_str(), nullptr));
		quatZ = static_cast<real>(strtod(Unicode::wideToNarrow(argv[9]).c_str(), nullptr));

		ServerObject *newObject = 0;
		ServerObjectTemplate const * const ot = getObjectTemplateForCreation(tmpString);
		if (ot)
		{
			if (ConfigServerGame::getStripNonFreeAssetsFromPlayersInTutorial() && NewbieTutorial::isInTutorialArea(userObject))
			{
				std::string sharedTemplate = ot->getSharedTemplate();

				if (!FileManifest::contains(sharedTemplate.c_str()))
				{
					result += getErrorMessage (argv[0], ERR_NOT_SKUFREE_TEMPLATE);
					ot->releaseReference();
					return true;
				}
			}
			
			Quaternion q(quatW, quatX, quatY, quatZ);
			Transform tr(Transform::IF_none);
			//get quaternion rotation as a transform
			q.getTransform(&tr);
			//complete transform with position
			tr.setPosition_p(x, y, z);
			//apply to object
			newObject = ServerWorld::createNewObject(*ot, tr, cell, false);
			checkBadBuildClusterObject(newObject);
			ot->releaseReference();
		}

		if (!newObject)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_TEMPLATE);
			return true;
		}
		else if (!cell)
		{
				newObject->addToWorld();
		}

		if (ConfigServerGame::getAdminPersistAllCreates())
		{
			newObject->persist();
		}
		else
		{
			newObject->setIncludeInBuildout( true );
		}

		result += Unicode::narrowToWide("NetworkId: ");
		result += Unicode::narrowToWide(newObject->getNetworkId().getValueString());
		result += Unicode::narrowToWide("\n");
		result += getErrorMessage (argv[0], ERR_SUCCESS);
	}
	else if (isCommand( argv[0], "getHeight"))
	{
		ServerObject * userObject = ServerWorld::findObjectByNetworkId(userId);
		if (!userObject)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_USER);
			return true;
		}
		Vector pos = userObject->getPosition_w();
		pos.y = 100000;
		real hgt = 0;
		const TerrainObject* terr = TerrainObject::getConstInstance();
		if (!terr->getHeight (pos, hgt))
		{
			result += getErrorMessage(argv[0], ERR_TERRAIN_GENERATION);
			return true;
		}
		char tmpBuf[256];
		sprintf(tmpBuf, "Height: %f\n", hgt);
		result += Unicode::narrowToWide(tmpBuf);
		return true;
	}
	else if (isCommand( argv[0], "getInventoryId"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject* object = dynamic_cast<ServerObject*>(NetworkIdManager::getObjectById(oid));
		if (object == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		SlottedContainer * const slotContainer = ContainerInterface::getSlottedContainer(*object);
		if (!slotContainer)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		const SlotId invenSlot = SlotIdManager::findSlotId(ConstCharCrcLowerString("inventory"));		
				
		Container::ContainerErrorCode ctmp = Container::CEC_Success;
		ServerObject* inventory = dynamic_cast<ServerObject*>(slotContainer->getObjectInSlot(invenSlot, ctmp).getObject());
		std::string text;
		if (!inventory)
		{
			text = "This Object has no inventory\n";
			result += Unicode::narrowToWide(text);
			return true;
		}
		char tmp[128];
		sprintf(tmp, "%s\n", inventory->getNetworkId().getValueString().c_str());
		text = tmp;
		result += Unicode::narrowToWide(text);
		result += getErrorMessage(argv[0], ERR_SUCCESS);
		return true;
	}

	else if (isCommand( argv[0], "getDatapadId"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject* object = dynamic_cast<ServerObject*>(NetworkIdManager::getObjectById(oid));
		if (object == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		SlottedContainer * const slotContainer = ContainerInterface::getSlottedContainer(*object);
		if (!slotContainer)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		const SlotId datapadSlot = SlotIdManager::findSlotId(ConstCharCrcLowerString("datapad"));		
		Container::ContainerErrorCode ctmp = Container::CEC_Success;
		ServerObject* datapad = dynamic_cast<ServerObject*>(slotContainer->getObjectInSlot(datapadSlot, ctmp).getObject());
		std::string text;
		if (!datapad)
		{
			text = "This Object has no datapad\n";
			result += Unicode::narrowToWide(text);
			return true;
		}
		char tmp[128];
		sprintf(tmp, "%s\n", datapad->getNetworkId().getValueString().c_str());
		text = tmp;
		result += Unicode::narrowToWide(text);
		result += getErrorMessage(argv[0], ERR_SUCCESS);
		return true;
	}
	
	//----------------------------------------------------------------------
	
	else if (isCommand( argv [0], "destroy"))
	{
		const NetworkId oid (Unicode::wideToNarrow(argv[1]));
		ServerObject* const object = safe_cast<ServerObject*>(NetworkIdManager::getObjectById(oid));
		
		if (object == nullptr)
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
		else if (object->getClient())
			result += getErrorMessage(argv[0], ERR_INVALID_USER);
		else 
		{
			ProsePackage pp;
			bool sendResponse = argv.size () > 2;
			
			if (sendResponse)
				ProsePackageManagerServer::createSimpleProsePackage (*object, SharedStringIds::rsp_object_deleted_prose, pp);
			
			if (object->permanentlyDestroy(DeleteReasons::God))
			{
				result += getErrorMessage(argv[0], ERR_SUCCESS);
				if (sendResponse)
					Chat::sendSystemMessage (*playerObject, pp);
			}
			else
				result += Unicode::narrowToWide("Internal Error encountered while deleting object");
		}		
		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand( argv [0], "destroyAllMobsExcept"))
	{
		const NetworkId oid (Unicode::wideToNarrow(argv[1]));

		ServerObject* const object = safe_cast<ServerObject*>(NetworkIdManager::getObjectById(oid));
		
		if (object == nullptr)
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);

		// ----------

		int objectCount = ServerWorld::getNumObjects();

		for(int i = 0; i < objectCount; i++)
		{
			ServerObject * object = ServerWorld::getObject(i);

			if(object == nullptr) continue;

			CreatureObject * creature = object->asCreatureObject();

			if(creature == nullptr) continue;

			if(creature->getNetworkId() == oid) continue;

			if(creature->getClient()) continue;

			creature->permanentlyDestroy(DeleteReasons::God);
		}

		result += Unicode::narrowToWide("Deleted all creatures but the given one");

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand( argv [0], "destroyAllWorldMobs"))
	{
		int objectCount = ServerWorld::getNumObjects();

		for(int i = 0; i < objectCount; i++)
		{
			ServerObject * object = ServerWorld::getObject(i);

			if(object == nullptr) continue;

			CreatureObject * creature = object->asCreatureObject();

			if(creature == nullptr) continue;

			if(creature->getParentCell() != CellProperty::getWorldCellProperty()) continue;

			if(creature->getClient()) continue;

			creature->permanentlyDestroy(DeleteReasons::God);
		}

		result += Unicode::narrowToWide("Deleted all world creatures");

		return true;
	}
	
	//-----------------------------------------------------------------
	
	else if (isCommand( argv[0], "move"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		Vector pos;
		pos.x = static_cast<real>(strtod(Unicode::wideToNarrow(argv[2]).c_str(), nullptr));
		pos.y = static_cast<real>(strtod(Unicode::wideToNarrow(argv[3]).c_str(), nullptr));
		pos.z = static_cast<real>(strtod(Unicode::wideToNarrow(argv[4]).c_str(), nullptr));

		// disallow certain object types from being "move"
		bool allowMove = true;
		ServerObject const *o = ServerWorld::findObjectByNetworkId(oid);
		if (o)
		{
			const char *objectTemplateName = o->getObjectTemplateName();
			if (objectTemplateName && (s_unmoveable.count(objectTemplateName) > 0))
			{
				char buffer[256];
				snprintf(buffer, sizeof(buffer)-1, "Object id cannot be %s\n", objectTemplateName);
				buffer[sizeof(buffer)-1] = '\0';

				result += Unicode::narrowToWide(buffer);
				result += getErrorMessage(argv[0], ERR_FAIL);
				allowMove = false;
			}
		}

		if (allowMove)
		{
			if (moveObject(oid, pos, NetworkId::cms_invalid, Vector()))
			{
				result += getErrorMessage(argv[0], ERR_SUCCESS);
			}
			else
			{
				result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			}
		}
	}
	
	//-----------------------------------------------------------------
	
	else if (isCommand( argv[0], "moveToMe"))
	{
		ServerObject * userObject = ServerWorld::findObjectByNetworkId(userId);
		if (userObject)
			userObject = safe_cast<ServerObject *>(ContainerInterface::getFirstParentInWorld(*userObject));
		if (!userObject)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_USER);
			return true;
		}
		NetworkId oid(Unicode::wideToNarrow(argv[1]));

		// disallow certain object types from being "moveToMe"
		bool allowMove = true;
		ServerObject const *o = ServerWorld::findObjectByNetworkId(oid);
		if (o)
		{
			const char *objectTemplateName = o->getObjectTemplateName();
			if (objectTemplateName && (s_unmoveable.count(objectTemplateName) > 0))
			{
				char buffer[256];
				snprintf(buffer, sizeof(buffer)-1, "Object id cannot be %s\n", objectTemplateName);
				buffer[sizeof(buffer)-1] = '\0';

				result += Unicode::narrowToWide(buffer);
				result += getErrorMessage(argv[0], ERR_FAIL);
				allowMove = false;
			}
		}

		if (allowMove)
		{
			Object *topmost = ContainerInterface::getTopmostContainer(*userObject);
			Object *containerObj = ContainerInterface::getContainedByObject(*userObject);
			if (moveObject(oid, topmost->getPosition_w(), containerObj ? containerObj->getNetworkId() : NetworkId::cms_invalid, userObject->getPosition_p()))
			{
				result += getErrorMessage(argv[0], ERR_SUCCESS);
			}
			else
			{
				result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			}
		}
	}
	//-----------------------------------------------------------------
	
	else if (isCommand( argv[0], "persist"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject* o = ServerWorld::findObjectByNetworkId(oid);
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		o->persist();
		result += getErrorMessage(argv[0], ERR_SUCCESS);		
	}
	
	//-----------------------------------------------------------------
	
	else if (isCommand( argv[0], "rotate"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		real r,p,y;
		r = static_cast<real>(strtod(Unicode::wideToNarrow(argv[2]).c_str(), nullptr));
		p = static_cast<real>(strtod(Unicode::wideToNarrow(argv[3]).c_str(), nullptr));
		y = static_cast<real>(strtod(Unicode::wideToNarrow(argv[4]).c_str(), nullptr));
		if (rotateObject(oid, r, p, y))
		{
			result += getErrorMessage(argv[0], ERR_SUCCESS);
		}
		else
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
		}
	}

	else if (isCommand( argv[0], "find"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject* o = ServerWorld::findObjectByNetworkId(oid);
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		Vector pos = o->getPosition_p();
		char locationString[94];
		
		const NetworkId & containedBy = ContainerInterface::getContainedByProperty(*o)->getContainedByNetworkId();
		
		sprintf(locationString, "Object %s: (%.1f %.1f %.1f) in %s", oid.getValueString().c_str(), pos.x, pos.y, pos.z, containedBy.getValueString().c_str());
		result += Unicode::narrowToWide(locationString);
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isCommand(argv[0], "findCreature"))
	{
		CreatureObject * const userObject = dynamic_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(userId));
		if (!userObject)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_USER);
			return true;
		}

		bool const openIfNotFound = true;
		DataTable * const dataTable = DataTableManager::getTable(s_creaureDataTable, openIfNotFound);

		// erase keys assigned to this player.
		s_playerCreatureNameMap.erase(userId);

		if (dataTable != nullptr)
		{
			StringVector creatureStrings;
			{
				std::vector<const char *> creatureNames;
				dataTable->getStringColumn("creatureName", creatureNames);

				std::vector<const char *>::const_iterator itName = creatureNames.begin();
				for (; itName != creatureNames.end(); ++itName)
				{
					creatureStrings.push_back(std::string(*itName));
				}
			}

			StringVector desiredSubStrings;
			for (int j = 1; j < (int)argv.size(); ++j)
				desiredSubStrings.push_back(Unicode::wideToNarrow(argv[j]));

			StringVector foundCreatureNames;
			result += Unicode::narrowToWide(searchNames(creatureStrings, desiredSubStrings, 0, foundCreatureNames, searchNamesCreatureFormatter));

			if (foundCreatureNames.size() > 0)
				s_playerCreatureNameMap.insert(std::make_pair(userId, foundCreatureNames));

			result += getErrorMessage(argv[0], ERR_SUCCESS);

			DataTableManager::close(s_creaureDataTable);
		}
	}
	else if (isCommand(argv[0], "findTemplate"))
	{
		CreatureObject * const userObject = dynamic_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(userId));
		if (!userObject)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_USER);
			return true;
		}

		// erase keys assigned to this player.
		s_playerObjectTemplateMap.erase(userId);

		StringVector objectTemplateStrings;
		{
			std::vector<const char *> objectTemplateNames;
			ObjectTemplateList::getAllTemplateNamesFromCrcStringTable(objectTemplateNames);

			std::vector<const char *>::const_iterator itName = objectTemplateNames.begin();
			for (; itName != objectTemplateNames.end(); ++itName)
				objectTemplateStrings.push_back(*itName);
		}

		result += Unicode::narrowToWide("Matching object template listing...\n");

		StringVector desiredTemplateSubStrings;
		for (int j = 1; j < (int)argv.size(); ++j)
			desiredTemplateSubStrings.push_back(Unicode::wideToNarrow(argv[j]));

		StringVector foundTemplateNames;
		result += Unicode::narrowToWide(searchNames(objectTemplateStrings, desiredTemplateSubStrings, "shared_", foundTemplateNames));

		if (foundTemplateNames.size() > 0)
			s_playerObjectTemplateMap.insert(std::make_pair(userId, foundTemplateNames));

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isCommand( argv[0], "findRelative_w"))
	{
		NetworkId const oid(Unicode::wideToNarrow(argv[1]));
		ServerObject* const o = ServerWorld::findObjectByNetworkId(oid);
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		Vector const & pos = playerObject->getPosition_w() - o->getPosition_w();
		
		char locationString[94];
		snprintf(locationString, sizeof(locationString), "Object %s: relative_w (%.1f %.1f %.1f) ", oid.getValueString().c_str(), pos.x, pos.y, pos.z);
		result += Unicode::narrowToWide(locationString);
	}
	else if (isCommand( argv[0], "fixHouseItemLimit"))
	{
		NetworkId const oid(Unicode::wideToNarrow(argv[1]));
		TangibleObject * const o = dynamic_cast<TangibleObject *>(ServerWorld::findObjectByNetworkId(oid));
		if (!o || !o->getPortalProperty())
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		o->fixUpPobItemLimit();
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isCommand( argv[0], "deletePersistedContents"))
	{
		NetworkId const oid(Unicode::wideToNarrow(argv[1]));
		TangibleObject * const o = dynamic_cast<TangibleObject *>(ServerWorld::findObjectByNetworkId(oid));
		if (!o || !o->getPortalProperty())
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		o->deletePobPersistedContents(nullptr, DeleteReasons::God);
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isCommand( argv[0], "moveItemInHouseToMe"))
	{
		CreatureObject * const userObject = dynamic_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(userId));
		if (!userObject)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_USER);
			return true;
		}

		NetworkId const oid(Unicode::wideToNarrow(argv[1]));
		TangibleObject * const o = dynamic_cast<TangibleObject *>(ServerWorld::findObjectByNetworkId(oid));
		if (!o || !o->getPortalProperty())
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		int index = atoi(Unicode::wideToNarrow(argv[2]).c_str());
		o->movePobItemToPlayer(*userObject, index, true); // Note: no need to check for gold oids here, because moveItemToPlayer() only succeeds if the house is player-placed
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	else if (isCommand( argv[0], "reinitializeVendor"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		TangibleObject* vendor = dynamic_cast<TangibleObject*>(ServerWorld::findObjectByNetworkId(oid));
		if (vendor == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		CommoditiesMarket::reinitializeVendor(*vendor, vendor->getOwnerId());
		
        result += getErrorMessage(argv[0], ERR_SUCCESS);
    }
    else if (isCommand( argv[0], "testVendorStatus"))
    {
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
        TangibleObject* vendor = dynamic_cast<TangibleObject*>(ServerWorld::findObjectByNetworkId(oid));
        if (vendor == nullptr)
        {
            result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
            return true;
        }
		int status  = atoi (Unicode::wideToNarrow (argv [2]).c_str ());
		CommoditiesMarket::vendorStatusChange(oid, status);
		
        result += getErrorMessage(argv[0], ERR_SUCCESS);
    }
	else if (isCommand(argv[0], "setAdminTitle"))
	{
		int which  = atoi (Unicode::wideToNarrow (argv [1]).c_str ());
		if (which < 0 || which > 3)
		{
			result += Unicode::narrowToWide("Invalid argument.  0=none, 1=csr, 2=dev, 3=qa");
			return true;
		}
		CreatureObject * userObject = dynamic_cast<CreatureObject*>(ServerWorld::findObjectByNetworkId(userId));
		if (!userObject)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_USER);
			return true;
		}
		PlayerObject* playerObject = PlayerCreatureController::getPlayerObject(userObject);
		if (!playerObject)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		switch(which)
		{
			case 0:
				playerObject->setPriviledgedTitle(static_cast<int8>(PlayerDataPriviledgedTitle::NormalPlayer));
				break;
			case 1:
				playerObject->setPriviledgedTitle(static_cast<int8>(PlayerDataPriviledgedTitle::CustomerServiceRepresentative));
				break;
			case 2:
				playerObject->setPriviledgedTitle(static_cast<int8>(PlayerDataPriviledgedTitle::Developer));
				break;
			case 3:
				playerObject->setPriviledgedTitle(static_cast<int8>(PlayerDataPriviledgedTitle::QualityAssurance));
				break;
			default:
				playerObject->setPriviledgedTitle(static_cast<int8>(PlayerDataPriviledgedTitle::NormalPlayer));
				break;
		}
		result += getErrorMessage(argv[0], ERR_SUCCESS);
		return true;
	}

	else if (isCommand(argv[0], "gotoObject"))
	{
			
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject* o = ServerWorld::findObjectByNetworkId(oid);
		if (!o)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		Object *topmost = ContainerInterface::getTopmostContainer(*o);
		Object *containerObj = ContainerInterface::getContainedByObject(*o);
		if (moveObject(userId, topmost->getPosition_w(), containerObj ? containerObj->getNetworkId() : NetworkId::cms_invalid, o->getPosition_p()))
		{
			result += getErrorMessage(argv[0], ERR_INVALID_USER);
			return true;
		}
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}

	//-----------------------------------------------------------------
	
	else if (isCommand( argv[0], "scaleMovement"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		CreatureObject* o = dynamic_cast<CreatureObject*>(ServerWorld::findObjectByNetworkId(oid));
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		real scale;
		scale = static_cast<real>(strtod(Unicode::wideToNarrow(argv[2]).c_str(), nullptr));
		o->setMovementScale(scale);
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}

	//-----------------------------------------------------------------

	else if (isCommand( argv[0], "scaleSize"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		CreatureObject* o = dynamic_cast<CreatureObject*>(ServerWorld::findObjectByNetworkId(oid));
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		real scale;
		scale = static_cast<real>(strtod(Unicode::wideToNarrow(argv[2]).c_str(), nullptr));
		o->setScaleFactor(scale);
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	//-----------------------------------------------------------------
	else if (isCommand(argv[0], "snap"))
	{
		const TerrainObject* terr = TerrainObject::getConstInstance();

		if (!terr)
		{
			//@todo need an error message for no terrain!
			result += getErrorMessage(argv[0], ERR_NO_TERRAIN);
			return true;
		}
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject* o = ServerWorld::findObjectByNetworkId(oid);
		if (!o || ContainerInterface::getContainedByObject(*o))
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		Vector pos = o->getPosition_p();
		pos.y = 100000;
		real hgt = 0;
		
		if (!terr->getHeight (pos, hgt))
		{
			//@todo need an error message for no terrain!
			result += getErrorMessage(argv[0], ERR_TERRAIN_GENERATION);
			return true;
		}
		pos.y = hgt;
		moveObject(oid, pos, NetworkId::cms_invalid, Vector()); //@todo...doing two oid lookups is inefficient.
		
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}
	//-----------------------------------------------------------------

	else if (isCommand(argv[0], "isProxy"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject* o = ServerWorld::findObjectByNetworkId(oid);
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		else
		{
			std::string text;
			if (!o->isAuthoritative())
			    text = "1";
			else
			    text = "0";
			result += Unicode::narrowToWide(text);
			result += getErrorMessage(argv[0], ERR_SUCCESS);
			return true;
			
		}
	}

	else if (isCommand(argv[0], "listContainer"))
	{
		CachedNetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * o = dynamic_cast<ServerObject *>(oid.getObject());
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		Container const * const container = ContainerInterface::getContainer(*o);
		if (container == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		std::string text;
		container->debugPrint(text);

		result += Unicode::narrowToWide(text);
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}

	else if(isCommand(argv[0], "triggerVolumes"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * o = ServerWorld::findObjectByNetworkId(oid);
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		else
		{
			ServerObject::TriggerVolumeMap const & volumes = o->getTriggerVolumeMap();
			char radius[1024];
			sprintf(radius, "%i", volumes.size());
			result = Unicode::narrowToWide("Listing ");
			result += Unicode::narrowToWide(radius);
			result += Unicode::narrowToWide(" trigger volumes for object ");
			result += argv[1].c_str();
			result += Unicode::narrowToWide(":\n");
			for (ServerObject::TriggerVolumeMap::const_iterator v = volumes.begin(); v != volumes.end(); ++v)
			{
				result += Unicode::narrowToWide("trigger volume for object ");
				result += argv[1].c_str();
				result += Unicode::narrowToWide(" ");
				result += Unicode::narrowToWide((*v).first.c_str());
				result += Unicode::narrowToWide(" ");
				sprintf(radius, "%f", (*v).second->getRadius());
				result += Unicode::narrowToWide(radius);
				result += Unicode::narrowToWide("\n");
			}
			result += Unicode::narrowToWide("Trigger Volume list for object ");
			result += argv[1].c_str();
			result += Unicode::narrowToWide("complete\n");
		}		
	}
	else if(isCommand(argv[0], "createTriggerVolume"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * o = ServerWorld::findObjectByNetworkId(oid);
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		else
		{
			const TriggerVolume * t = o->getTriggerVolume(Unicode::wideToNarrow(argv[2]));
			if(t == 0)
			{
				const std::string name = Unicode::wideToNarrow(argv[2]);
				const float radius = static_cast<float>(atof(Unicode::wideToNarrow(argv[3].c_str()).c_str()));
				o->createTriggerVolume(radius, name, true);
				result = Unicode::narrowToWide("Created a new trigger volume named ");
				result += argv[2];
				result += Unicode::narrowToWide(" with a radius of ");
				result += argv[3];
				result += Unicode::narrowToWide(" on object ");
				result += argv[1];
				result += Unicode::narrowToWide("\n");
			}
			else
			{
				result = Unicode::narrowToWide("A trigger volume named ");
				result += argv[2];
				result += Unicode::narrowToWide(" already exists on object ");
				result += argv[1];
				result += Unicode::narrowToWide("\n");
			}
		}
	}
	else if(isCommand(argv[0], "removeTriggerVolume"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * o = ServerWorld::findObjectByNetworkId(oid);
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		else
		{
			const TriggerVolume * t = o->getTriggerVolume(Unicode::wideToNarrow(argv[2]));
			if(t != 0)
			{
				const std::string name = Unicode::wideToNarrow(argv[2]);
				o->removeTriggerVolume(name);
				result = Unicode::narrowToWide("Removed trigger volume named ");
				result += argv[2];
				result += Unicode::narrowToWide(" from object ");
				result += argv[1];
				result += Unicode::narrowToWide("\n");
			}
			else
			{
				result = Unicode::narrowToWide("A trigger volume named ");
				result += argv[2];
				result += Unicode::narrowToWide(" does not exists on object ");
				result += argv[1];
				result += Unicode::narrowToWide("\n");
			}
		}
	}
	
	else if (isCommand(argv[0], "setName"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		TangibleObject* o = dynamic_cast<TangibleObject *>(ServerWorld::findObjectByNetworkId(oid));
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		else if (o->isPlayerControlled())
		{
			result += Unicode::narrowToWide("You cannot use this command on a player");
			return true;
		}
		else
		{
			o->setObjectName(argv[2]);
			result += getErrorMessage(argv[0], ERR_SUCCESS);
		}
		
		return true;
		
	}
	
	//-----------------------------------------------------------------

	else if (isCommand(argv[0], "getServerTemplate"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		const ServerObject* o = dynamic_cast<const ServerObject *>(ServerWorld::findObjectByNetworkId(oid));
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		else
		{
			result += argv[1];
			result += Unicode::narrowToWide(" ");
			result += Unicode::narrowToWide(o->getTemplateName());
			result += Unicode::narrowToWide("\n");
			result += getErrorMessage(argv[0], ERR_SUCCESS);
		}

		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand(argv[0], "reloadServerTemplate"))
	{
		std::string tmpString(Unicode::wideToNarrow(argv[1]));
		Iff templateFile;
		if (!templateFile.open(tmpString.c_str(), true))
		{
			result += getErrorMessage (argv[0], ERR_INVALID_TEMPLATE);
			return true;
		}

		ServerMessageForwarding::beginBroadcast();

		ReloadTemplateMessage const reloadTemplateMessage(tmpString);
		ServerMessageForwarding::send(reloadTemplateMessage);

		ServerMessageForwarding::end();

		if (ObjectTemplateList::reload(templateFile) == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_TEMPLATE_NOT_LOADED);
			return true;
		}
		result += getErrorMessage(argv[0], ERR_SUCCESS);
		return true;
	}

	//-----------------------------------------------------------------

	else if (isCommand(argv[0], "isIncapacitated"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		CreatureObject * creature = dynamic_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(oid));
		if (creature == nullptr || !creature->isIncapacitated())
			result += Unicode::narrowToWide("no");
		else if (creature->isDead())
			result += Unicode::narrowToWide("dead");
		else
			result += Unicode::narrowToWide("incapacitated");
		return true;
	}

	//-----------------------------------------------------------------------

	else if(isCommand(argv[0], "addLocationTarget"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		TangibleObject * o = dynamic_cast<TangibleObject *>(ServerWorld::findObjectByNetworkId(oid));
		if(o)
		{
			LocationData d;
			d.name = argv[2];
			Vector pos;
			pos.x = static_cast<real>(strtod(Unicode::wideToNarrow(argv[3]).c_str(), nullptr));
			pos.y = static_cast<real>(strtod(Unicode::wideToNarrow(argv[4]).c_str(), nullptr));
			pos.z = static_cast<real>(strtod(Unicode::wideToNarrow(argv[5]).c_str(), nullptr));
			d.location.setCenter(pos);
			float radius = static_cast<real>(strtod(Unicode::wideToNarrow(argv[6]).c_str(), nullptr));
			d.location.setRadius(radius);
			o->addLocationTarget(d);			
		}
		else
		{
			result += Unicode::narrowToWide("Object does not exist or is not derived from TangibleObject");
		}
	}

	//-----------------------------------------------------------------------

	else if(isCommand(argv[0], "removeLocationTarget"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		TangibleObject * o = dynamic_cast<TangibleObject *>(ServerWorld::findObjectByNetworkId(oid));
		if(o)
		{
			o->removeLocationTarget(argv[2]);
		}
	}

	//-----------------------------------------------------------------

	else if(isCommand(argv[0], "watchAIPath"))
	{
		RequestWatchObjectPath rwop;
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		bool enable = strtoul(Unicode::wideToNarrow(argv[2]).c_str(), 0, 10) != 0;
		rwop.setClientId(userId);
		rwop.setObjectId(NetworkId(oid));
		rwop.setEnable(enable);
		emitMessage(rwop);
	}

	//-----------------------------------------------------------------

	else if(isCommand(argv[0], "watchPathMap"))
	{
		RequestWatchPathMap rwpm;
		bool enable = strtoul(Unicode::wideToNarrow(argv[1]).c_str(), 0, 10) != 0;
		rwpm.setClientId(userId);
		rwpm.setEnable(enable);
		emitMessage(rwpm);
	}

	//-----------------------------------------------------------------

	else if(isCommand(argv[0], "unstick"))
	{
		RequestUnstick request;
		request.setClientId(userId);
		emitMessage(request);
	}

	//-----------------------------------------------------------------

	else if(isCommand(argv[0], "attribs"))
	{
		CachedNetworkId oid(Unicode::wideToNarrow(argv[1]));
		const TangibleObject * tangible = dynamic_cast<const TangibleObject *>(
			oid.getObject());
		const CreatureObject * creature = dynamic_cast<const CreatureObject *>(
			tangible);
		if (creature != nullptr)
		{
			char buffer[1024];
			sprintf(buffer, "he:%d, co=%d, ac=%d, st=%d, mi=%d, wi=%d", 
					creature->getAttribute(Attributes::Health),
					creature->getAttribute(Attributes::Constitution),
					creature->getAttribute(Attributes::Action),
					creature->getAttribute(Attributes::Stamina),
					creature->getAttribute(Attributes::Mind),
					creature->getAttribute(Attributes::Willpower));
			result += Unicode::narrowToWide(buffer);
		}
		else if (tangible != nullptr)
		{
			char buffer[1024];
			sprintf(buffer, "max hp = %d, damage taken = %d",
					tangible->getMaxHitPoints(),
					tangible->getDamageTaken());
			result += Unicode::narrowToWide(buffer);
		}
		else
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
	}

	//-----------------------------------------------------------------------

	else if (isCommand(argv[0], "count"))
	{
		char buf[32] = {"\0"};
		snprintf(buf, sizeof(buf), "%lu objects allocated\n", ServerObject::getObjectCount());
		result += Unicode::narrowToWide(std::string(buf));
	}

	//-----------------------------------------------------------------

	else if (isCommand(argv[0], "enqueueCommand"))
	{
		CachedNetworkId actorId(Unicode::wideToNarrow(argv[1]));
		CachedNetworkId targetId(Unicode::wideToNarrow(argv[3]));

		CreatureObject *actorObj = dynamic_cast<CreatureObject*>(ServerWorld::findObjectByNetworkId(actorId));
		if (actorObj)
		{
			Command const &command = CommandTable::getCommand(Crc::normalizeAndCalculate(Unicode::wideToNarrow(argv[2]).c_str()));
			if (!command.isNull())
				actorObj->commandQueueEnqueue(command, targetId, argv[4]);
			else
				result += getErrorMessage(argv[0], ERR_FAIL);
		}
		else
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
	}

	//-----------------------------------------------------------------
	else if (isCommand(argv[0], "hide"))
	{
		NetworkId actorId(Unicode::wideToNarrow(argv[1]));
		bool value = strtoul(Unicode::wideToNarrow(argv[2]).c_str(), 0, 10) != 0;

		TangibleObject *actorObj = dynamic_cast<TangibleObject *>(ServerWorld::findObjectByNetworkId(actorId));
		if (actorObj)
		{
			actorObj->setVisible(!value);
			result += getErrorMessage (argv[0], ERR_SUCCESS);
		}
		else
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
	}

	//-----------------------------------------------------------------

	else if (isCommand(argv[0], "setCoverVisibility"))
	{
		NetworkId actorId(Unicode::wideToNarrow(argv[1]));
		bool value = strtoul(Unicode::wideToNarrow(argv[2]).c_str(), 0, 10) != 0;

		ServerObject * const o = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(actorId));
		CreatureObject * const c = (o ? o->asCreatureObject() : nullptr);
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		if (c == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a creature object\n");
			return true;
		}

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("setting creature object (%s) cover visibility to (%s)\n", actorId.getValueString().c_str(), (value ? "true" : "false")));
		c->setCoverVisibility(value);
	}

	//-----------------------------------------------------------------

	else if(isCommand(argv[0], "getExposedProxyList") || isCommand(argv[0], "getPrivateProxyList"))
	{
		bool const usePrivateList = isCommand(argv[0], "getPrivateProxyList");

		CachedNetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * obj = dynamic_cast<ServerObject *>(oid.getObject());
		if(obj)
		{
			ProxyList const &proxyList = usePrivateList ? obj->getPrivateProxyList() : obj->getExposedProxyList();
			char objbuf[1024];
			if (usePrivateList)
				snprintf(objbuf, sizeof(objbuf) - 1, "Private proxy list for (%s : %s):\n", obj->getObjectTemplateName(), oid.getValueString().c_str());
			else
				snprintf(objbuf, sizeof(objbuf) - 1, "Exposed proxy list for (%s : %s):\n", obj->getObjectTemplateName(), oid.getValueString().c_str());
			objbuf[sizeof(objbuf) - 1] = '\0';

			result += Unicode::narrowToWide(std::string(objbuf));
			for (ProxyList::const_iterator i = proxyList.begin(); i != proxyList.end(); ++i)
			{
				char numbuf[16] = {"\0"};
				snprintf(numbuf, sizeof(numbuf), "%li", (*i));
				result += Unicode::narrowToWide(std::string(numbuf));
				result += Unicode::narrowToWide(std::string("\n"));
			}
		}
		else
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
		}
	}

	// ----------------------------------------------------------------------

	else
	{
		// ConsoleCommandParserObject::performParsing() is getting too big for the compiler to handle
		// so some of the message handling is done in the private helper performParsing2()
		return performParsing2(userId, argv, originalCommand, result, node);
	}

	return true;
}

// ---------------------------------------------------------------------
// ConsoleCommandParserObject::performParsing() is getting too big for the compiler to handle
// so some of the message handling is done in this private helper performParsing2()

bool ConsoleCommandParserObject::performParsing2(const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser *)
{
	CreatureObject * const playerObject = dynamic_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(userId));
	if (!playerObject)
	{
		WARNING_STRICT_FATAL(true, ("Console command executed on invalid player object %s", userId.getValueString().c_str()));
		return false;
	}

	UNREF(originalCommand);

	if(isCommand(argv[0], "getObservers"))
	{
		CachedNetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * obj = dynamic_cast<ServerObject *>(oid.getObject());
		if (obj)
		{
			std::string observers;
			const std::set<Client *>& observerList = obj->getObservers();
			std::set<Client *>::const_iterator j;
			for (j = observerList.begin(); j != observerList.end(); ++j)
			{
				const Client* observerClient = (*j);
				if (observerClient)
				{
					observers += observerClient->getCharacterObjectId().getValueString();
					observers += " (";

					ServerObject const* observer = observerClient->getCharacterObject();
					if (observer)
					{
						observers += Unicode::wideToNarrow(observer->getAssignedObjectName()).c_str();
					}

					observers += ")\n";
				}
			}

			const char *objectTemplateName = obj->getObjectTemplateName();

			result += Unicode::narrowToWide(FormattedString<512>().sprintf("object %s (%s) has %d total observers (%d observers on this game server)\n", obj->getNetworkId().getValueString().c_str(), (objectTemplateName ? objectTemplateName : "nullptr object template"), obj->getObserversCount(), observerList.size()));

			if (!observers.empty())
			{
				result += Unicode::narrowToWide(observers);
			}
		}
		else
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
		}
	}

	// ----------------------------------------------------------------------

	else if(isCommand(argv[0], "getObserving"))
	{
		CachedNetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * obj = dynamic_cast<ServerObject *>(oid.getObject());
		if (obj && obj->getClient())
		{
			Client::ObservingList const &observing = obj->getClient()->getObserving();
			char objbuf[1024] = {"\0"};
			snprintf(objbuf, sizeof(objbuf), "Observing list (%d items) for %s:\n", observing.size(), oid.getValueString().c_str());
			result += Unicode::narrowToWide(std::string(objbuf));
			int count = 1;
			for (Client::ObservingList::const_iterator i = observing.begin(); i != observing.end(); ++i)
			{
				snprintf(objbuf, sizeof(objbuf), "%d: ", count++);
				result += Unicode::narrowToWide(std::string(objbuf));

				result += Unicode::narrowToWide((*i)->getNetworkId().getValueString());
				result += Unicode::narrowToWide(std::string("\n"));
			}
		}
		else
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
		}
	}

	// ----------------------------------------------------------------------

	else if(isCommand(argv[0], "getObservingDetailed"))
	{
		CachedNetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * obj = dynamic_cast<ServerObject *>(oid.getObject());
		if (obj && obj->getClient())
		{
			Client::ObservingList const &observing = obj->getClient()->getObserving();
			char objbuf[1024] = {"\0"};
			snprintf(objbuf, sizeof(objbuf), "Observing list (%d items) for %s:\n", observing.size(), oid.getValueString().c_str());
			result += Unicode::narrowToWide(std::string(objbuf));
			int count = 1;
			for (Client::ObservingList::const_iterator i = observing.begin(); i != observing.end(); ++i)
			{
				snprintf(objbuf, sizeof(objbuf), "%d: ", count++);
				result += Unicode::narrowToWide(std::string(objbuf));

				result += Unicode::narrowToWide((*i)->getNetworkId().getValueString());

				const char *objectTemplateName = (*i)->getObjectTemplateName();
				if (objectTemplateName)
				{
					result += Unicode::narrowToWide(std::string(", "));
					result += Unicode::narrowToWide(std::string(objectTemplateName));
				}

				result += Unicode::narrowToWide(std::string("\n"));
			}
		}
		else
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
		}
	}

	// ----------------------------------------------------------------------

	else if(isCommand(argv[0], "getObservingPvpSync"))
	{
		CachedNetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * obj = dynamic_cast<ServerObject *>(oid.getObject());
		if (obj && obj->getClient())
		{
			Client::ObservingListPvpSync const &observing = obj->getClient()->getObservingPvpSync();
			char objbuf[1024] = {"\0"};
			snprintf(objbuf, sizeof(objbuf), "Observing (pvp sync) list (%d items) for %s:\n", observing.size(), oid.getValueString().c_str());
			result += Unicode::narrowToWide(std::string(objbuf));
			int count = 1;
			for (Client::ObservingListPvpSync::const_iterator i = observing.begin(); i != observing.end(); ++i)
			{
				snprintf(objbuf, sizeof(objbuf), "%d: ", count++);
				result += Unicode::narrowToWide(std::string(objbuf));

				result += Unicode::narrowToWide((*i)->getNetworkId().getValueString());
				result += Unicode::narrowToWide(std::string("\n"));
			}
		}
		else
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
		}
	}

	// ----------------------------------------------------------------------

	else if(isCommand(argv[0], "getObservingPvpSyncDetailed"))
	{
		CachedNetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * obj = dynamic_cast<ServerObject *>(oid.getObject());
		if (obj && obj->getClient())
		{
			Client::ObservingListPvpSync const &observing = obj->getClient()->getObservingPvpSync();
			char objbuf[1024] = {"\0"};
			snprintf(objbuf, sizeof(objbuf), "Observing (pvp sync) list (%d items) for %s:\n", observing.size(), oid.getValueString().c_str());
			result += Unicode::narrowToWide(std::string(objbuf));
			int count = 1;
			for (Client::ObservingListPvpSync::const_iterator i = observing.begin(); i != observing.end(); ++i)
			{
				snprintf(objbuf, sizeof(objbuf), "%d: ", count++);
				result += Unicode::narrowToWide(std::string(objbuf));

				result += Unicode::narrowToWide((*i)->getNetworkId().getValueString());

				const char *objectTemplateName = (*i)->getObjectTemplateName();
				if (objectTemplateName)
				{
					result += Unicode::narrowToWide(std::string(", "));
					result += Unicode::narrowToWide(std::string(objectTemplateName));
				}

				result += Unicode::narrowToWide(std::string("\n"));
			}
		}
		else
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
		}
	}

	//------------------------------------------------------------------------------------------

	else if(isCommand(argv[0], "getVolume"))
	{
		CachedNetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * obj = dynamic_cast<ServerObject *>(oid.getObject());
		if (obj)
		{
			VolumeContainmentProperty* prop = ContainerInterface::getVolumeContainmentProperty(*obj);
			if (prop)
			{
				char tmp[50];
				snprintf(tmp, 50, "%d\n", prop->getVolume());
				result += Unicode::narrowToWide(tmp);
			}
		}
	}

	// ----------------------------------------------------------------------
	else if(isCommand(argv[0], "getLoadWith"))
	{
		CachedNetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * obj = dynamic_cast<ServerObject *>(oid.getObject());
		if (obj)
		{
			char tmp[50];
			NetworkId loadWithId = ContainerInterface::getLoadWithContainerId(*obj);
			snprintf(tmp, 50, "Value: %s, result: %s\n", obj->getLoadWith().getValueString().c_str(), loadWithId.getValueString().c_str());
			result += Unicode::narrowToWide(tmp);
		}
	}

	// ----------------------------------------------------------------------

	else if(isCommand(argv[0], "renamePlayer"))
	{
		const NetworkId oid(Unicode::wideToNarrow(argv[1]));
		if (!NameManager::getInstance().isPlayer(oid))
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s is not a player character\n", oid.getValueString().c_str()));
		}
		else
		{
			PlayerCreationManagerServer::renamePlayer(static_cast<int8>(RenameCharacterMessageEx::RCMS_console_god_command),NameManager::getInstance().getPlayerStationId(oid),oid,argv[2],Unicode::narrowToWide(NameManager::getInstance().getPlayerFullName(oid)),userId);
			result += getErrorMessage (argv[0], ERR_SUCCESS);
		}
	}

	//-----------------------------------------------------------------

	else if(isCommand(argv[0], "loadContents"))
	{
		const CachedNetworkId container(Unicode::wideToNarrow(argv[1]));
		const ServerObject *obj=dynamic_cast<const ServerObject*>(container.getObject());
		if (!obj)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
		}
		else
		{
			if (argv.size() >= 3)
			{
				NetworkId oid(Unicode::wideToNarrow(argv[2]));
				if (oid!=NetworkId::cms_invalid)
				{
					obj->loadContainedObjectFromDB(oid);
					result += getErrorMessage (argv[0], ERR_SUCCESS);
				}
				else
					result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);				
			}
			else
			{
				obj->loadAllContentsFromDB();
				result += getErrorMessage (argv[0], ERR_SUCCESS);
			}
		}
	}

	// ----------------------------------------------------------------------

	else if(isCommand(argv[0], "getAttributes"))
	{
		CachedNetworkId container(Unicode::wideToNarrow(argv[1]));
		const ServerObject *obj=dynamic_cast<const ServerObject*>(container.getObject());
		if (!obj)
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
		else
		{
			std::vector<std::pair<std::string, Unicode::String> > data;
			obj->getAttributes(userId, data);
			for (std::vector<std::pair<std::string, Unicode::String> >::iterator i=data.begin(); i!=data.end(); ++i)
			{
				result += Unicode::narrowToWide(i->first + ":  ") + i->second + Unicode::narrowToWide("\n");
			}
		}
	}

	// ----------------------------------------------------------------------

	else if(isCommand(argv[0], "createPathNodes"))
	{
		NetworkId const oid(Unicode::wideToNarrow(argv[1]));

		ServerObject * object = ServerWorld::findObjectByNetworkId(oid);

		if(CityPathGraphManager::createPathNodes(object))
			result += getErrorMessage (argv[0], ERR_SUCCESS);
		else
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
	}

	// ----------------------------------------------------------------------

	else if(isCommand(argv[0], "destroyPathNodes"))
	{
		NetworkId const oid(Unicode::wideToNarrow(argv[1]));

		ServerObject * object = ServerWorld::findObjectByNetworkId(oid);

		if(CityPathGraphManager::destroyPathNodes(object))
			result += getErrorMessage (argv[0], ERR_SUCCESS);
		else
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
	}

	// ----------------------------------------------------------------------

	else if(isCommand(argv[0], "reloadPathNodes"))
	{
		if(CityPathGraphManager::reloadPathNodes())
			result += getErrorMessage (argv[0], ERR_SUCCESS);
		else
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
	}

	// ----------------------------------------------------------------------

	else if(isCommand(argv[0], "relinkPathGraph"))
	{
		CityPathGraph const * graph = CityPathGraphManager::getCityGraphFor(playerObject);

		if(graph)
		{
			CityPathGraphManager::relinkGraph(graph);

			result += getErrorMessage (argv[0], ERR_SUCCESS);
		}
		else
		{
			result += getErrorMessage (argv[0], ERR_FAIL);
		}
	}

	// ----------------------------------------------------------------------

	else if(isCommand(argv[0], "setPathLinkDistance"))
	{
		float dist = static_cast<real>(strtod(Unicode::wideToNarrow(argv[1]).c_str(), nullptr));

		CityPathGraphManager::setLinkDistance(dist);

		result += getErrorMessage (argv[0], ERR_SUCCESS);
	}

	// ----------------------------------------------------------------------

	else if(isCommand(argv[0], "markCityEntrance"))
	{
		NetworkId const oid(Unicode::wideToNarrow(argv[1]));

		ServerObject * object = ServerWorld::findObjectByNetworkId(oid);

		if(CityPathGraphManager::markCityEntrance(object))
			result += getErrorMessage (argv[0], ERR_SUCCESS);
		else
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
	}

	// ----------------------------------------------------------------------

	else if (isCommand(argv[0], "unmarkCityEntrance"))
	{
		NetworkId const oid(Unicode::wideToNarrow(argv[1]));

		ServerObject * object = ServerWorld::findObjectByNetworkId(oid);

		if(CityPathGraphManager::unmarkCityEntrance(object))
			result += getErrorMessage (argv[0], ERR_SUCCESS);
		else
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
	}

	// ----------------------------------------------------------------------

	else if (isCommand(argv[0], "setOwner"))
	{
		CachedNetworkId const oid(Unicode::wideToNarrow(argv[1]));
		TangibleObject * object = dynamic_cast<TangibleObject *>(oid.getObject());
		if (object == nullptr)
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
		else
		{
			NetworkId ownerId(playerObject->getNetworkId());
			if (argv.size() == 3)
			{
				ownerId = NetworkId(Unicode::wideToNarrow(argv[2]));
			}
			object->setOwnerId(ownerId);
			result += getErrorMessage (argv[0], ERR_SUCCESS);
		}
	}

	else if (isCommand(argv[0], "getOwner"))
	{
		CachedNetworkId const oid(Unicode::wideToNarrow(argv[1]));
		TangibleObject * object = dynamic_cast<TangibleObject *>(oid.getObject());
		if (object == nullptr)
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
		else
		{
			NetworkId ownerId = object->getOwnerId();
			result += Unicode::narrowToWide("Owner is: ");
			result += Unicode::narrowToWide(ownerId.getValueString());
			result += Unicode::narrowToWide("\n");
			result += getErrorMessage (argv[0], ERR_SUCCESS);
		}
	}
	// ----------------------------------------------------------------------

	else if (isCommand(argv[0], "askConsent"))
	{
		ProsePackage p;
		p.stringId = StringId("ui", "testaskconsent");
		ConsentManager::getInstance().askConsent(userId, p, &ConsoleCommandParserObjectNamespace::testConsentHandler);
	}

	// ----------------------------------------------------------------------

	else if(isCommand(argv[0], "debugFar"))
	{
		CachedNetworkId oid(Unicode::wideToNarrow(argv[1]));
		bool create = strtoul(Unicode::wideToNarrow(argv[2]).c_str(), 0, 10) != 0;
		
		float overrideVal = 0.0f;
		if(argv.size() > 3)
			overrideVal = static_cast<float>(atof(Unicode::wideToNarrow(argv[3]).c_str()));


		ServerObject * object = dynamic_cast<TangibleObject *>(oid.getObject());
		CreatureObject * creature = dynamic_cast<CreatureObject *>(object);
		if (object == nullptr)
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
		else
		{
			if(create && creature)
			{
				creature->createFarNetworkUpdateVolume(overrideVal);
			}
			else if (create)
			{
				object->createFarNetworkUpdateVolume();
			}
			else
				object->destroyFarNetworkUpdateVolume();
		}

	}

	// ----------------------------------------------------------------------

	else if (isCommand(argv[0], "multiplyBuildingRadius"))
	{
		float const multiplier = static_cast<float>(atof (Unicode::wideToNarrow (argv [1]).c_str ()));
		ServerObject::setBuildingUpdateRadiusMultiplier(multiplier);
		result += getErrorMessage (argv[0], ERR_SUCCESS);		
	}


	// ----------------------------------------------------------------------

	else if(isCommand(argv[0], "addJediSlot"))
	{
		Client * client = playerObject->getClient();
		if (!client->isGod())
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		CreatureObject *creature  = dynamic_cast<CreatureObject*>(NetworkIdManager::getObjectById(oid));
		PlayerObject *player = nullptr;
		if (creature)
			player = PlayerCreatureController::getPlayerObject(creature);
		if (player)
		{
			player->addJediToAccount();
			result += getErrorMessage (argv[0], ERR_SUCCESS);
		}
		else
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);			
	}

	else if(isCommand(argv[0], "getJediSlotInfo"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		CreatureObject const * creature  = dynamic_cast<CreatureObject const *>(NetworkIdManager::getObjectById(oid));
		if (creature)
		{
			Client const * client = creature->getClient();
			if (client)
			{
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s, hasUnoccupiedJediSlot=%s, isJediSlotCharacter=%s\n", PlayerObject::getAccountDescription(creature).c_str(), (client->getHasUnoccupiedJediSlot() ? "yes" : "no"), (client->getIsJediSlotCharacter() ? "yes" : "no")));
			}
			else
			{
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("object %s doesn't have a Client object (possible causes are may not be a player character object, may not be authoritative, or may not be connected)\n", oid.getValueString().c_str()));
			}
		}
		else
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);	
		}
	}

	else if(isCommand(argv[0], "makeSpectral"))
	{
		Client * client = playerObject->getClient();
		if (!client->isGod())
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		CreatureObject *creature  = dynamic_cast<CreatureObject*>(NetworkIdManager::getObjectById(oid));
		PlayerObject *player = nullptr;
		if (creature)
			player = PlayerCreatureController::getPlayerObject(creature);
		if (player)
		{
			player->makeSpectralOnAccount();
			result += getErrorMessage (argv[0], ERR_SUCCESS);
		}
		else
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);			
	}

	// ----------------------------------------------------------------------

	else if(isCommand(argv[0], "getHibernating"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		CreatureObject *creature  = dynamic_cast<CreatureObject*>(NetworkIdManager::getObjectById(oid));

		if(creature)
		{
			AICreatureController * controller = dynamic_cast<AICreatureController*>(creature->getController());

			if(controller)
			{
				if(controller->getHibernate())
				{
					result += Unicode::narrowToWide("true\n");
				}
				else
				{
					result += Unicode::narrowToWide("false\n");
				}

				result += getErrorMessage(argv[0], ERR_SUCCESS);
			}
			else
			{
				result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			}
		}
		else
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
		}
	}

	// ----------------------------------------------------------------------

	else if(isCommand(argv[0], "getBehavior"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		CreatureObject *creature  = dynamic_cast<CreatureObject*>(NetworkIdManager::getObjectById(oid));

		if(creature)
		{
			AICreatureController * controller = dynamic_cast<AICreatureController*>(creature->getController());

			if(controller)
			{
				std::string temp;

				controller->getBehaviorDebugInfo(temp);

				result += Unicode::narrowToWide(temp.c_str());
				result += Unicode::narrowToWide("\n");
				result += getErrorMessage(argv[0], ERR_SUCCESS);
			}
			else
			{
				result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			}
		}
		else
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
		}
	}

	// ----------------------------------------------------------------------

	else if(isCommand(argv[0], "getStandingOn"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		CreatureObject *creature  = dynamic_cast<CreatureObject*>(NetworkIdManager::getObjectById(oid));

		if(creature)
		{
			CollisionProperty const * collision = creature->getCollisionProperty();

			if(collision)
			{
				Object const * object = collision->getStandingOn();

				if(object)
				{
					NetworkId const & id = object->getNetworkId();

					result += Unicode::narrowToWide(id.getValueString().c_str());
					result += Unicode::narrowToWide("\n");
					result += getErrorMessage(argv[0], ERR_SUCCESS);
				}
				else
				{
					result += Unicode::narrowToWide("Creature is not standing on anyting\n");
					result += getErrorMessage(argv[0], ERR_SUCCESS);
				}
			}
			else
			{
				result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			}
		}
		else
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
		}
	}

	// ----------------------------------------------------------------------

	else if(isCommand(argv[0], "getPower"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		const InstallationObject* o = dynamic_cast<const InstallationObject*>(ServerWorld::findObjectByNetworkId(oid));
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		char buffer[32];
		sprintf(buffer, "%.1f\n", o->getPower());
		result += Unicode::narrowToWide(buffer);
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}

	// ----------------------------------------------------------------------

	else if(isCommand(argv[0], "setPower"))
	{
		Client * client = playerObject->getClient();
		if (!client->isGod())
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		InstallationObject* o = dynamic_cast<InstallationObject*>(ServerWorld::findObjectByNetworkId(oid));
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		float power = static_cast<float>(atof(Unicode::wideToNarrow(argv[2].c_str()).c_str()));
		o->setPower(power);
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}

	// ----------------------------------------------------------------------

	else if(isCommand(argv[0], "getPowerRate"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		const InstallationObject* o = dynamic_cast<const InstallationObject*>(ServerWorld::findObjectByNetworkId(oid));
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		char buffer[32];
		sprintf(buffer, "%.1f\n", o->getPowerRate());
		result += Unicode::narrowToWide(buffer);
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}

	// ----------------------------------------------------------------------

	else if(isCommand(argv[0], "setPowerRate"))
	{
		Client * client = playerObject->getClient();
		if (!client->isGod())
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		InstallationObject* o = dynamic_cast<InstallationObject*>(ServerWorld::findObjectByNetworkId(oid));
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		float powerRate = static_cast<float>(atof(Unicode::wideToNarrow(argv[2].c_str()).c_str()));
		o->setPowerRate(powerRate);
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}

	else if(isCommand(argv[0], "getBattlefieldMarkerRegionName"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		BattlefieldMarkerObject *o = dynamic_cast<BattlefieldMarkerObject *>(ServerWorld::findObjectByNetworkId(oid));
		if (!o)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		result += Unicode::narrowToWide(o->getRegionName());
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}

	else if(isCommand(argv[0], "setBattlefieldMarkerRegionName"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		BattlefieldMarkerObject *o = dynamic_cast<BattlefieldMarkerObject *>(ServerWorld::findObjectByNetworkId(oid));
		if (!o)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		o->setRegionName(Unicode::wideToNarrow(argv[2]));
		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}

	else if(isCommand(argv[0], "unload"))
	{
		// Unload an object on all servers.  Tell *every* gameserver to unload it, since this is
		// normally to be used to correct objects in a bad state.
		NetworkId oid(Unicode::wideToNarrow(argv[1]));

		ServerObject * const o = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(oid));
		if (o)
			o->unload();

		ServerMessageForwarding::beginBroadcast();

		UnloadObjectMessage const unloadObjectMessage(oid);
		ServerMessageForwarding::send(unloadObjectMessage);

		ServerMessageForwarding::end();

		result += getErrorMessage(argv[0], ERR_SUCCESS);
	}

	// ----------------------------------------------------------------------

	else if(isCommand(argv[0], "moveToPlayerAndUnload"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		NetworkId player(Unicode::wideToNarrow(argv[2]));

		if (oid < ConfigServerGame::getMaxGoldNetworkId())
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
		else
		{
			ServerObject *o = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(oid));
			if (o && !dynamic_cast<UniverseObject*>(o) && !o->asCreatureObject() && !dynamic_cast<IntangibleObject*>(o))
			{
				const ServerObjectTemplate * const targetTemplate = safe_cast<const ServerObjectTemplate *>(o->getObjectTemplate());

				bool canMoveInWorld = false;
				if (targetTemplate)
				{
					for (size_t i = 0; i < targetTemplate->getMoveFlagsCount(); ++i)
					{
						if (targetTemplate->getMoveFlags(i) == ServerObjectTemplate::MF_player)
						{
							canMoveInWorld = true;
						}
					}
				}

				if (canMoveInWorld && o->getCacheVersion() == 0)
				{
					// disallow certain object types from being "moveToPlayerAndUnload"
					const char *objectTemplateName = o->getObjectTemplateName();
					if (objectTemplateName && (s_unmoveable.count(objectTemplateName) > 0))
					{
						char buffer[256];
						snprintf(buffer, sizeof(buffer)-1, "Object id cannot be %s\n", objectTemplateName);
						buffer[sizeof(buffer)-1] = '\0';

						result += Unicode::narrowToWide(buffer);
						result += getErrorMessage(argv[0], ERR_FAIL);
					}
					else
					{
						o->moveToPlayerAndUnload(player);
						result += getErrorMessage(argv[0], ERR_SUCCESS);
						LOG("CustomerService", ("Player:object %s is being given to offline player %s by %s", o->getNetworkId().getValueString().c_str(), player.getValueString().c_str(), PlayerObject::getAccountDescription(userId).c_str()));
					}
				}
				else
				{
					result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
				}
			}
			else
				result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
		}
	}

	// ----------------------------------------------------------------------

	else if(isCommand(argv[0], "resetStats"))
	{
		// make sure the person using the command is an admin
		Client * client = playerObject->getClient();
		if (!client->isGod())
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject *o = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(oid));
		if (o != nullptr && o->asCreatureObject() != nullptr && o->isPlayerControlled())
		{
			CreatureObject * creatureTarget = o->asCreatureObject();

			// find the 1st skill the player has that is a profession
			std::string professionName;
			const CreatureObject::SkillList & skills = creatureTarget->getSkillList();
			if (!skills.empty())
			{
				for (CreatureObject::SkillList::const_iterator i = skills.begin(); i != skills.end(); ++i)
				{
					const SkillObject * profession = (*i)->findProfessionForSkill();
					if (profession != nullptr)
						professionName = profession->getSkillName();
				}
			}
			if (professionName.empty())
			{
				// just get the 1st profession available
				PlayerCreationManager::StringVector professions;
				PlayerCreationManager::getProfessionVector (professions, "");
				if (professions.empty())
				{
					WARNING(true, ("Could not find any profession handling "
						"resetStats command"));
					result += getErrorMessage(argv[0], ERR_FAIL);
					return true;
				}
				professionName = professions[0];
			}

			std::vector<int> racialModifiers;
			std::vector<int> professionModifiers;

			if (!PlayerCreationManager::getRacialModifiers(creatureTarget->getSharedTemplateName(), racialModifiers))
			{
				WARNING(true, ("Could not get racial modifiers for template "
					"%s handling resetStats command",
					creatureTarget->getSharedTemplateName()));
				result += getErrorMessage(argv[0], ERR_FAIL);
				return true;
			}
			if (!PlayerCreationManager::getProfessionModifiers(professionName.c_str(), 
				professionModifiers))
			{
				WARNING(true, ("Could not get profession modifiers for "
					"profession %s handling resetStats command",
					professionName.c_str()));
				result += getErrorMessage(argv[0], ERR_FAIL);
				return true;
			}
			if (racialModifiers.size() != professionModifiers.size() ||
				racialModifiers.size() != static_cast<size_t>(Attributes::NumberOfAttributes))
			{
				WARNING(true, ("Invalid racial (%d) or profession (%d) data "
					"handling resetStats command", racialModifiers.size(),
					professionModifiers.size()));
				result += getErrorMessage(argv[0], ERR_FAIL);
				return true;
			}
			for (int i = 0; i < Attributes::NumberOfAttributes; ++i)
			{
				creatureTarget->resetAttribute(i, static_cast<Attributes::Value>(
					racialModifiers[i] + professionModifiers[i]));
			}
			result += getErrorMessage(argv[0], ERR_SUCCESS);
		}
		else
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
	}

	// ----------------------------------------------------------------------

	else if(isCommand(argv[0], "restoreHouse"))
	{
		NetworkId houseId(Unicode::wideToNarrow(argv[1]));

		GenericValueTypeMessage<std::pair<NetworkId, std::string> > const msg("RestoreHouseMessage", std::make_pair(houseId, Unicode::wideToNarrow(playerObject->getAssignedObjectFirstName())));
		GameServer::getInstance().sendToDatabaseServer(msg);

		result += Unicode::narrowToWide("Request sent.  Please wait for reply.  This may take several minutes if a save cycle is in progress.");
	}

	// ----------------------------------------------------------------------

	else if(isCommand(argv[0], "restoreCharacter"))
	{
		NetworkId characterId(Unicode::wideToNarrow(argv[1]));

		GenericValueTypeMessage<std::pair<NetworkId, std::string> > const msg("RestoreCharacterMessage", std::make_pair(characterId, Unicode::wideToNarrow(playerObject->getAssignedObjectFirstName())));
		GameServer::getInstance().sendToDatabaseServer(msg);

		result += Unicode::narrowToWide("Request sent.  Please wait for reply.  This may take several minutes if a save cycle is in progress.");
	}

	// ----------------------------------------------------------------------

	else if(isCommand(argv[0], "enableCharacter"))
	{
		StationId stationId = static_cast<StationId>(atoi (Unicode::wideToNarrow (argv [1]).c_str ()));
		NetworkId playerId(Unicode::wideToNarrow(argv[2]));

		GenericValueTypeMessage<std::pair<std::pair<StationId, NetworkId>, std::string> > msg("EnableCharacterMessage", std::make_pair(std::make_pair(stationId, playerId), Unicode::wideToNarrow(playerObject->getAssignedObjectFirstName())));
		GameServer::getInstance().sendToCentralServer(msg);

		result += Unicode::narrowToWide("Request sent.  Please wait for reply.");
	}

	// ----------------------------------------------------------------------

	else if(isCommand(argv[0], "disableCharacter"))
	{
		StationId stationId = static_cast<StationId>(atoi (Unicode::wideToNarrow (argv [1]).c_str ()));
		NetworkId playerId(Unicode::wideToNarrow(argv[2]));

		GenericValueTypeMessage<std::pair<std::pair<StationId, NetworkId>, std::string> > msg("DisableCharacterMessage", std::make_pair(std::make_pair(stationId, playerId), Unicode::wideToNarrow(playerObject->getAssignedObjectFirstName())));
		GameServer::getInstance().sendToCentralServer(msg);

		result += Unicode::narrowToWide("Request sent.  Please wait for reply.");
	}

	// ----------------------------------------------------------------------

	else if(isCommand(argv[0], "loginCharacter"))
	{
		NetworkId const playerId(Unicode::wideToNarrow(argv[1]));

		// get the StationId for the specified NetworkId
		StationId const stationId = NameManager::getInstance().getPlayerStationId(playerId);
		if (stationId == 0)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("There is no character with id (%s) on this cluster\n", playerId.getValueString().c_str()));
		}
		else
		{
			std::string const & sourceCharacterName = NameManager::getInstance().getPlayerFullName(playerId);

			TransferCharacterData data(TransferRequestMoveValidation::TRS_transfer_server);
			data.setCharacterId(playerId);
			data.setSourceStationId(stationId);
			data.setSourceCharacterName(sourceCharacterName);
			data.setSourceGalaxy(GameServer::getInstance().getClusterName());
			data.setCSToolId(GameServer::getInstance().getProcessId());
			const GenericValueTypeMessage<TransferCharacterData> loginCharacter("TransferGetLoginLocationData", data);
			GameServer::getInstance().sendToDatabaseServer(loginCharacter);

			result += Unicode::narrowToWide(FormattedString<512>().sprintf("Request to login character (%s, %s), station id (%lu) sent\n", playerId.getValueString().c_str(), sourceCharacterName.c_str(), stationId));
		}
	}

	// ----------------------------------------------------------------------

	else if(isCommand(argv[0], "undeleteItem"))
	{
		NetworkId itemId(Unicode::wideToNarrow(argv[1]));
		GenericValueTypeMessage<std::pair<NetworkId, std::string> > msg("UndeleteItemMessage", std::make_pair(itemId, Unicode::wideToNarrow(playerObject->getAssignedObjectFirstName())));
		GameServer::getInstance().sendToDatabaseServer(msg);

		result += Unicode::narrowToWide("Request sent.  Please wait for reply.  This may take several minutes if a save cycle is in progress.");
	}

	// ----------------------------------------------------------------------

	else if(isCommand(argv[0], "loadStructure"))
	{
		NetworkId itemId(Unicode::wideToNarrow(argv[1]));

		LocateStructureMessage msg(itemId, 0, 0, "", Unicode::wideToNarrow(playerObject->getAssignedObjectFirstName()));
		GameServer::getInstance().sendToDatabaseServer(msg);
		result += Unicode::narrowToWide("Request sent.  Please wait for reply.  This may take several minutes if a save cycle is in progress.");
	}

	// ----------------------------------------------------------------------

	else if(isCommand(argv[0], "deleteAuctionLocation"))
	{
		NetworkId locationId(Unicode::wideToNarrow(argv[1]));

		CommoditiesMarket::deleteAuctionLocation(locationId, Unicode::wideToNarrow(playerObject->getAssignedObjectFirstName()));

		result += Unicode::narrowToWide("Request sent.  Please wait for reply.");
	}

	// ----------------------------------------------------------------------

	else if(isCommand(argv[0], "moveToPlayer"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		NetworkId player(Unicode::wideToNarrow(argv[2]));

		if (oid < ConfigServerGame::getMaxGoldNetworkId())
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
		else
		{
			GenericValueTypeMessage<std::pair<std::pair<NetworkId, NetworkId>, std::string> > msg("MoveToPlayerMessage", std::make_pair(std::make_pair(oid, player), Unicode::wideToNarrow(playerObject->getAssignedObjectFirstName())));
			GameServer::getInstance().sendToDatabaseServer(msg);

			result += Unicode::narrowToWide("Request sent.  Please wait for reply.  This may take several minutes if a save cycle is in progress.");
		}
	}

	// ----------------------------------------------------------------------

	else if(isCommand(argv[0], "fixLoadWith"))
	{
		NetworkId topmostObject(Unicode::wideToNarrow(argv[1]));
		NetworkId startingLoadWith(Unicode::wideToNarrow(argv[2]));
		int maxDepth = atoi(Unicode::wideToNarrow(argv[3]).c_str());

		if (topmostObject < ConfigServerGame::getMaxGoldNetworkId() || startingLoadWith < ConfigServerGame::getMaxGoldNetworkId())
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
		else
		{
			GenericValueTypeMessage<std::pair<std::pair<NetworkId, NetworkId>, int> > msg("FixLoadWith",
				std::make_pair(std::make_pair(topmostObject, startingLoadWith), maxDepth));
			GameServer::getInstance().sendToDatabaseServer(msg);

			result += Unicode::narrowToWide("Request sent.  Please wait for reply.  This may take several minutes if a save cycle is in progress.");
		}
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "moveSimStart"))
	{
		int countNpc    = 1;
		int countPc     = 1;
		float radius    = 20.0f;
		float moveSpeed = 1.0f;

		if (argv.size () > 3)
		{
			countNpc  = atoi (Unicode::wideToNarrow (argv [1]).c_str ());
			countPc   = atoi (Unicode::wideToNarrow (argv [2]).c_str ());
			radius    = static_cast<float>(atof (Unicode::wideToNarrow (argv [3]).c_str ()));
			moveSpeed = static_cast<float>(atof (Unicode::wideToNarrow (argv [4]).c_str ()));
		}

		MoveSimManager::start (userId, countNpc, countPc, radius, moveSpeed);
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], "moveSimClear"))
	{
		MoveSimManager::clear ();
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "playUiEffect"))
	{
		if (argv.size() < 2)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_PARAMETER_COUNT);
			return false;
		}

		NetworkId oid (Unicode::wideToNarrow(argv[1]));
		ServerObject* const object = safe_cast<ServerObject*>(NetworkIdManager::getObjectById(oid));

		if (object == nullptr)
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
		else if (!object->getClient())
			result += getErrorMessage(argv[0], ERR_INVALID_USER);
		else 
		{
			Unicode::String resultString;
			static Unicode::String const spaceString = Unicode::narrowToWide(" ");

			for(StringVector_t::const_iterator itString = (argv.begin() + 2); itString != argv.end(); ++itString)
			{
				resultString += *itString;
				resultString += spaceString;
			}

			//format the message.
			MessageQueueGenericValueType<std::pair<NetworkId, std::string> > * msg = new MessageQueueGenericValueType<std::pair<NetworkId, std::string> >(std::make_pair(oid, Unicode::wideToNarrow(resultString)));
			object->appendMessage(CM_uiPlayEffect, 0.0f, msg);
		}		
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "rollupStructure"))
	{
		NetworkId houseId (Unicode::wideToNarrow(argv[1]));
		NetworkId ownerId (Unicode::wideToNarrow(argv[2]));

		ServerObject * const object = safe_cast<ServerObject*>(NetworkIdManager::getObjectById(houseId));
		TangibleObject * const tangible = object ? object->asTangibleObject() : nullptr;
		if (!tangible)
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
		else
			tangible->rollupStructure(ownerId, false);
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "regionList"))
	{
		ServerObject * object = safe_cast<ServerObject*>(NetworkIdManager::getObjectById(playerObject->getLookAtTarget()));

		if (nullptr == object)
			object = playerObject;

		RegionMaster::RegionVector rv;
		std::string const & planetName = ServerWorld::getSceneId();
		Vector const & pos_w = object->getPosition_w();

		RegionMaster::getRegionsAtPoint(planetName, pos_w.x, pos_w.z, rv);

		char buf[64];

		for (RegionMaster::RegionVector::const_iterator it = rv.begin(); it != rv.end(); ++it)
		{
			Region const * const r = *it;
			if (nullptr != r)
			{
				_itoa(r->getGeography(), buf, 10);
				result += Unicode::narrowToWide(buf);
				result += Unicode::narrowToWide(" \\>32 ");
				result += r->getName();
				result += Unicode::narrowToWide("\\>000\n");
			}
		}
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], "canMove"))
	{
		ServerObject * const target = safe_cast<ServerObject*>(NetworkIdManager::getObjectById(playerObject->getLookAtTarget()));

		if (nullptr == target)
		{
			result += Unicode::narrowToWide("Invalid target");
			return true;
		}

		Vector const & goal_w = target->getPosition_w();

		CanMoveResult cmr = CollisionWorld::canMove(playerObject, goal_w, 1.0f, false, false, false);

		result += Unicode::narrowToWide("result=");

		switch (cmr)
		{
		case CMR_MoveOK:
			result += Unicode::narrowToWide("CMR_MoveOK");
			break;
		case CMR_HitFloorEdge:
			result += Unicode::narrowToWide("CMR_HitFloorEdge");
			break;
		case CMR_HitObstacle:
			result += Unicode::narrowToWide("CMR_HitObstacle");
			break;
		case CMR_MissedGoal:
			result += Unicode::narrowToWide("CMR_MissedGoal");
			break;
		case CMR_Error:
			result += Unicode::narrowToWide("CMR_Error");
			break;
		case CMR_Invalid:
			result += Unicode::narrowToWide("CMR_Invalid");
			break;
		}
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], "pathAutoGenerate"))
	{
		float const nodeDistance = static_cast<float>(atof(Unicode::wideToNarrow(argv[1]).c_str()));
		float const obstacleDistance = static_cast<float>(atof(Unicode::wideToNarrow(argv[2]).c_str()));

		ServerObject * object = safe_cast<ServerObject*>(NetworkIdManager::getObjectById(playerObject->getLookAtTarget()));

		if (nullptr == object)
			object = playerObject;

		Vector const & pos_w = object->getPosition_w();

		PathAutoGenerator::pathAutoCleanup(pos_w, result);
		PathAutoGenerator::pathAutoGenerate(pos_w, nodeDistance, obstacleDistance, result);
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv [0], "pathAutoCleanup"))
	{
		ServerObject * object = safe_cast<ServerObject*>(NetworkIdManager::getObjectById(playerObject->getLookAtTarget()));

		if (nullptr == object)
			object = playerObject;

		Vector const & pos_w = object->getPosition_w();

		PathAutoGenerator::pathAutoCleanup(pos_w, result);
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "pathAutoCleanup"))
	{
		ServerObject * object = safe_cast<ServerObject*>(NetworkIdManager::getObjectById(playerObject->getLookAtTarget()));

		if (nullptr == object)
			object = playerObject;

		Vector const & pos_w = object->getPosition_w();

		PathAutoGenerator::pathAutoCleanup(pos_w, result);
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "getStaticInfo"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		const ServerObject* o = dynamic_cast<const ServerObject *>(ServerWorld::findObjectByNetworkId(oid));
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		else
		{
			result += argv[1];
			result += Unicode::narrowToWide(" ");
			result += Unicode::narrowToWide(o->getStaticItemName());
			char tmpBuf[256];
			sprintf(tmpBuf, ":%d\n", o->getStaticItemVersion());
			result += Unicode::narrowToWide(tmpBuf);
			result += Unicode::narrowToWide("\n");
			result += getErrorMessage(argv[0], ERR_SUCCESS);
		}

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "getBroadcastInfo"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		const ServerObject* o = dynamic_cast<const ServerObject *>(ServerWorld::findObjectByNetworkId(oid));
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		else
		{
			typedef std::map<std::string, std::set<NetworkId> > BroadcastMap;

			result += Unicode::narrowToWide("listening to:\n");
			BroadcastMap broadcastMap;

			{
				ServerObject::BroadcastMap const & broadcasters = o->getBroadcastBroadcasters();

				for (ServerObject::BroadcastMap::const_iterator i = broadcasters.begin(); i != broadcasters.end(); ++i)
					IGNORE_RETURN(broadcastMap[i->first].insert(i->second));

				for (BroadcastMap::const_iterator j = broadcastMap.begin(); j != broadcastMap.end(); ++j)
				{
					result += Unicode::narrowToWide(j->first);
					result += Unicode::narrowToWide(":");

					for (std::set<NetworkId>::const_iterator k = j->second.begin(); k != j->second.end(); ++k)
					{
						if (k != j->second.begin())
							result += Unicode::narrowToWide(" ");

						result += Unicode::narrowToWide(k->getValueString());
					}

					result += Unicode::narrowToWide("\n");
				}
			}

			result += Unicode::narrowToWide("broadcasting to:\n");
			broadcastMap.clear();

			{
				ServerObject::BroadcastMap const & listeners = o->getBroadcastListeners();

				for (ServerObject::BroadcastMap::const_iterator i = listeners.begin(); i != listeners.end(); ++i)
					IGNORE_RETURN(broadcastMap[i->first].insert(i->second));

				for (BroadcastMap::const_iterator j = broadcastMap.begin(); j != broadcastMap.end(); ++j)
				{
					result += Unicode::narrowToWide(j->first);
					result += Unicode::narrowToWide(":");

					for (std::set<NetworkId>::const_iterator k = j->second.begin(); k != j->second.end(); ++k)
					{
						if (k != j->second.begin())
							result += Unicode::narrowToWide(" ");

						result += Unicode::narrowToWide(k->getValueString());
					}

					result += Unicode::narrowToWide("\n");
				}
			}

			result += getErrorMessage(argv[0], ERR_SUCCESS);
		}

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "getAge"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject const* o = dynamic_cast<ServerObject const*>(ServerWorld::findObjectByNetworkId(oid));
		CreatureObject const* c = (o ? o->asCreatureObject() : nullptr);
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		else if (c == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a creature object\n");
			return true;
		}
		else
		{
			PlayerObject const* p = PlayerCreatureController::getPlayerObject(c);
			if (p == nullptr)
			{
				result += Unicode::narrowToWide("specified object is not a character object\n");
				return true;
			}
			else
			{
				int const currentBornDate = PlayerObject::getCurrentBornDate();
				int const bornDate = p->getBornDate();
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("character object %s (%s) is (current born date = %d, %s) - (born date = %d, %s) = (%d days old)\n", oid.getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str(), currentBornDate, CalendarTime::getCharacerBirthDateString(currentBornDate).c_str(), bornDate, CalendarTime::getCharacerBirthDateString(bornDate).c_str(), (currentBornDate - bornDate)));
			}
		}

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "adjustAge"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject* o = dynamic_cast<ServerObject*>(ServerWorld::findObjectByNetworkId(oid));
		CreatureObject* c = (o ? o->asCreatureObject() : nullptr);
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		else if (!o->isAuthoritative())
		{
			result += Unicode::narrowToWide("specified object is not authoritative on this game server\n");
			return true;
		}
		else if (c == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a creature object\n");
			return true;
		}
		else
		{
			PlayerObject* p = PlayerCreatureController::getPlayerObject(c);
			if (p == nullptr)
			{
				result += Unicode::narrowToWide("specified object is not a character object\n");
				return true;
			}
			else
			{
				int const adjustment = atoi(Unicode::wideToNarrow(argv[2]).c_str());
				int const existingBornDate = p->getBornDate();
				int const todayBornDate = PlayerObject::getCurrentBornDate();
				int const currentAge = todayBornDate - existingBornDate;
				int const newBornDate = existingBornDate - adjustment; // to adjust age, must adjust the born date appropriately

				if ((newBornDate <= 0) || (newBornDate > todayBornDate))
				{
					result += Unicode::narrowToWide(FormattedString<512>().sprintf("cannot adjust character object %s (%s) (%d days old) age by %d days because that would make the character too young or too old\n", oid.getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str(), currentAge, adjustment));
					return true;
				}
				else
				{
					p->adjustBornDate(-adjustment); // to adjust age, must adjust the born date appropriately

					const int newAge = todayBornDate - p->getBornDate();
					result += Unicode::narrowToWide(FormattedString<512>().sprintf("character object %s (%s) age has been adjusted from %d days old to %d days old\n", oid.getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str(), currentAge, newAge));
				}
			}
		}

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "getBadge"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * o = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(oid));
		CreatureObject * c = (o ? o->asCreatureObject() : nullptr);
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		else if (c == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a creature object\n");
			return true;
		}

		PlayerObject * p = PlayerCreatureController::getPlayerObject(c);
		if (p == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a character object\n");
			return true;
		}

		// earned badges
		std::vector<CollectionsDataTable::CollectionInfoSlot const *> earnedBadges;
		IGNORE_RETURN(p->getCompletedCollectionSlotCountInBook("badge_book", &earnedBadges));

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("%d Earned Badges:\n", earnedBadges.size()));

		std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iter;
		for (iter = earnedBadges.begin(); iter != earnedBadges.end(); ++iter)
		{
			result += Unicode::narrowToWide((*iter)->name);
			result += Unicode::narrowToWide("|");
			result += CollectionsDataTable::localizeCollectionName((*iter)->name);

			if (!(*iter)->titles.empty())
			{
				result += Unicode::narrowToWide("|");

				for (std::vector<std::string>::const_iterator iterTitle = (*iter)->titles.begin(); iterTitle != (*iter)->titles.end(); ++iterTitle)
				{
					if (iterTitle != (*iter)->titles.begin())
						result += Unicode::narrowToWide(", ");

					result += CollectionsDataTable::localizeCollectionTitle(*iterTitle);
				}
			}

			result += Unicode::narrowToWide("\n");
		}

		// unearned badges
		std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & allBadges = CollectionsDataTable::getSlotsInBook("badge_book");

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("\n%d Unearned Badges:\n", (allBadges.size() - earnedBadges.size())));

		for (iter = allBadges.begin(); iter != allBadges.end(); ++iter)
		{
			if (!p->hasCompletedCollectionSlot(**iter))
			{
				result += Unicode::narrowToWide((*iter)->name);
				result += Unicode::narrowToWide("|");
				result += CollectionsDataTable::localizeCollectionName((*iter)->name);

				if (!(*iter)->titles.empty())
				{
					result += Unicode::narrowToWide("|");

					for (std::vector<std::string>::const_iterator iterTitle = (*iter)->titles.begin(); iterTitle != (*iter)->titles.end(); ++iterTitle)
					{
						if (iterTitle != (*iter)->titles.begin())
							result += Unicode::narrowToWide(", ");

						result += CollectionsDataTable::localizeCollectionTitle(*iterTitle);
					}
				}

				result += Unicode::narrowToWide("\n");
			}
		}

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "getPassiveReveal"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject const* o = dynamic_cast<ServerObject const*>(ServerWorld::findObjectByNetworkId(oid));
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		TangibleObject const* t = o->asTangibleObject();
		if (t == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a tangible object\n");
			return true;
		}

		std::map<NetworkId, int> const & passiveReveal = t->getPassiveReveal();
		std::set<NetworkId> const & passiveRevealPlayerCharacter = t->getPassiveRevealPlayerCharacter();

		if (passiveReveal.empty())
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s has an empty passive reveal list\n", t->getNetworkId().getValueString().c_str()));
		}
		else
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("Displaying passive reveal list (size=%d) for %s\n", passiveReveal.size(), t->getNetworkId().getValueString().c_str()));

			for (std::map<NetworkId, int>::const_iterator iter = passiveReveal.begin(); iter != passiveReveal.end(); ++iter)
			{
				if (passiveRevealPlayerCharacter.count(iter->first) > 0)
					result += Unicode::narrowToWide(FormattedString<512>().sprintf("oid=%s (PC), range=%d\n", iter->first.getValueString().c_str(), iter->second));
				else
					result += Unicode::narrowToWide(FormattedString<512>().sprintf("oid=%s (NPC), range=%d\n", iter->first.getValueString().c_str(), iter->second));
			}
		}

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "addPassiveReveal"))
	{
		NetworkId sourceOid(Unicode::wideToNarrow(argv[1]));
		ServerObject * sourceSo = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(sourceOid));
		if (sourceSo == nullptr)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s is an invalid object\n", sourceOid.getValueString().c_str()));
			return true;
		}

		TangibleObject * sourceTo = sourceSo->asTangibleObject();
		if (sourceTo == nullptr)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s is not a tangible object\n", sourceOid.getValueString().c_str()));
			return true;
		}

		NetworkId targetOid(Unicode::wideToNarrow(argv[2]));
		ServerObject * targetSo = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(targetOid));
		if (targetSo == nullptr)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s is an invalid object\n", targetOid.getValueString().c_str()));
			return true;
		}

		TangibleObject * targetTo = targetSo->asTangibleObject();
		if (targetTo == nullptr)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s is not a tangible object\n", targetOid.getValueString().c_str()));
			return true;
		}

		int const range = atoi(Unicode::wideToNarrow(argv[3]).c_str());

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("Updating %s's passive reveal list: adding %s, range=%d\n", sourceTo->getNetworkId().getValueString().c_str(), targetTo->getNetworkId().getValueString().c_str(), range));
		sourceTo->addPassiveReveal(*targetTo, range);

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "removePassiveReveal"))
	{
		NetworkId sourceOid(Unicode::wideToNarrow(argv[1]));
		ServerObject * sourceSo = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(sourceOid));
		if (sourceSo == nullptr)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s is an invalid object\n", sourceOid.getValueString().c_str()));
			return true;
		}

		TangibleObject * sourceTo = sourceSo->asTangibleObject();
		if (sourceTo == nullptr)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s is not a tangible object\n", sourceOid.getValueString().c_str()));
			return true;
		}

		NetworkId targetOid(Unicode::wideToNarrow(argv[2]));

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("Updating %s's passive reveal list: removing %s\n", sourceTo->getNetworkId().getValueString().c_str(), targetOid.getValueString().c_str()));
		sourceTo->removePassiveReveal(targetOid);

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "clearPassiveReveal"))
	{
		NetworkId sourceOid(Unicode::wideToNarrow(argv[1]));
		ServerObject * sourceSo = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(sourceOid));
		if (sourceSo == nullptr)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s is an invalid object\n", sourceOid.getValueString().c_str()));
			return true;
		}

		TangibleObject * sourceTo = sourceSo->asTangibleObject();
		if (sourceTo == nullptr)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s is not a tangible object\n", sourceOid.getValueString().c_str()));
			return true;
		}

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("Clearing %s's passive reveal list\n", sourceTo->getNetworkId().getValueString().c_str()));
		sourceTo->removeAllPassiveReveal();

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "getInvulnerabilityTimer"))
	{
		NetworkId sourceOid(Unicode::wideToNarrow(argv[1]));
		ServerObject const * sourceSo = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(sourceOid));
		if (sourceSo == nullptr)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s is an invalid object\n", sourceOid.getValueString().c_str()));
			return true;
		}

		CreatureObject const * sourceCo = sourceSo->asCreatureObject();
		if (sourceCo == nullptr)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s is not a creature object\n", sourceOid.getValueString().c_str()));
			return true;
		}

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s's invulnerability timer is %.10f\n", sourceCo->getNetworkId().getValueString().c_str(), sourceCo->getInvulnerabilityTimer()));

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "spewCommandQueue"))
	{
		NetworkId sourceOid(Unicode::wideToNarrow(argv[1]));
		ServerObject * sourceSo = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(sourceOid));
		if (sourceSo == nullptr)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s is an invalid object\n", sourceOid.getValueString().c_str()));
			return true;
		}

		CreatureObject * sourceCo = sourceSo->asCreatureObject();
		if (sourceCo == nullptr)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s is not a creature object\n", sourceOid.getValueString().c_str()));
			return true;
		}

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("dumping %s's command queue contents\n", sourceCo->getNetworkId().getValueString().c_str()));

		CommandQueue * queue = sourceCo->getCommandQueue();
		if (queue != nullptr)
		{
			std::string output;
			queue->spew(&output);
			result += Unicode::narrowToWide(output);
		}

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "getChatStatistics"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * o = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(oid));
		CreatureObject * c = (o ? o->asCreatureObject() : nullptr);
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}
		else if (c == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a creature object\n");
			return true;
		}

		PlayerObject * p = PlayerCreatureController::getPlayerObject(c);
		if (p == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a character object\n");
			return true;
		}

		time_t const timeNow = ::time(nullptr);
		time_t const timeStartInterval = ((p->getChatSpamTimeEndInterval() > 0) ? static_cast<time_t>(p->getChatSpamTimeEndInterval() - (ConfigServerGame::getChatSpamLimiterIntervalMinutes() * 60)) : 0);

		if ((timeStartInterval <= 0) || (timeNow < timeStartInterval))
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("chat statistics for character object %s (%s) since %s:\n", oid.getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str(), CalendarTime::convertEpochToTimeStringLocal(timeStartInterval).c_str()));
		else
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("chat statistics for character object %s (%s) since %s (%s ago):\n", oid.getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str(), CalendarTime::convertEpochToTimeStringLocal(timeStartInterval).c_str(), CalendarTime::convertSecondsToMS(static_cast<unsigned int>(timeNow - timeStartInterval)).c_str()));

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("spatial character count: %d\n", p->getChatSpamSpatialNumCharacters()));
		result += Unicode::narrowToWide(FormattedString<512>().sprintf("non-spatial character count: %d (excluding tell and group chat)\n", p->getChatSpamNonSpatialNumCharacters()));

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "adjustSwgAccountFeatureId"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject const * o = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(oid));
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		CreatureObject const * c = o->asCreatureObject();
		if (c == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a creature object\n");
			return true;
		}

		PlayerObject const * p = PlayerCreatureController::getPlayerObject(c);
		if (p == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a character object\n");
			return true;
		}

		Client * client = o->getClient();
		if (client == nullptr)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("specified character object %s (%s) doesn't have a Client object (possible causes are may not be authoritative, or may not be connected)\n", o->getNetworkId().getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str()));
			return true;
		}

		uint32 const featureId = static_cast<uint32>(::atoi(Unicode::wideToNarrow(argv[2]).c_str()));
		int const adjustment = ::atoi(Unicode::wideToNarrow(argv[3]).c_str());

		if (featureId == 0)
		{
			result += Unicode::narrowToWide("feature Id value must be non-zero\n");
			return true;
		}

		if (adjustment == 0)
		{
			result += Unicode::narrowToWide("adjustment value must be non-zero\n");
			return true;
		}

		result += Unicode::narrowToWide(FormattedString<1024>().sprintf("requesting adjustment of (%d) for SWG feature Id (%lu) on character object %s (%s) account (%lu)... please wait for reply.\n", adjustment, featureId, o->getNetworkId().getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str(), p->getStationId()));

		AdjustAccountFeatureIdRequest const msg(userId, GameServer::getInstance().getProcessId(), o->getNetworkId(), std::string(), p->getStationId(), NetworkId::cms_invalid, std::string(), PlatformGameCode::SWG, featureId, adjustment);
		client->sendToConnectionServer(msg);

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "reloadSwgAccountFeatureId"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * o = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(oid));
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		CreatureObject const * c = o->asCreatureObject();
		if (c == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a creature object\n");
			return true;
		}

		PlayerObject const * p = PlayerCreatureController::getPlayerObject(c);
		if (p == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a character object\n");
			return true;
		}

		Client * client = o->getClient();
		if (client == nullptr)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("specified character object %s (%s) doesn't have a Client object (possible causes are may not be authoritative, or may not be connected)\n", o->getNetworkId().getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str()));
			return true;
		}

		result += Unicode::narrowToWide(FormattedString<1024>().sprintf("requesting SWG account feature Id for character object %s (%s) account (%lu)... please wait for reply.\n", o->getNetworkId().getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str(), p->getStationId()));

		AccountFeatureIdRequest const req(userId, GameServer::getInstance().getProcessId(), oid, p->getStationId(), PlatformGameCode::SWG, AccountFeatureIdRequest::RR_ConsoleCommandReloadRequest);
		client->sendToConnectionServer(req);

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "reloadSwgTcgAccountFeatureId"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * o = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(oid));
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		CreatureObject const * c = o->asCreatureObject();
		if (c == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a creature object\n");
			return true;
		}

		PlayerObject const * p = PlayerCreatureController::getPlayerObject(c);
		if (p == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a character object\n");
			return true;
		}

		Client * client = o->getClient();
		if (client == nullptr)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("specified character object %s (%s) doesn't have a Client object (possible causes are may not be authoritative, or may not be connected)\n", o->getNetworkId().getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str()));
			return true;
		}

		result += Unicode::narrowToWide(FormattedString<1024>().sprintf("requesting SWG TCG account feature Id for character object %s (%s) account (%lu)... please wait for reply.\n", o->getNetworkId().getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str(), p->getStationId()));

		AccountFeatureIdRequest const req(userId, GameServer::getInstance().getProcessId(), oid, p->getStationId(), PlatformGameCode::SWGTCG, AccountFeatureIdRequest::RR_ConsoleCommandReloadRequest);
		client->sendToConnectionServer(req);

		return true;
	}

	// ----------------------------------------------------------------------

	else if(isCommand(argv[0], "unloadBuildingContents"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		bool unloaded = false;

		ServerObject * const o = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(oid));
		if (o)
		{
			BuildingObject * const building = o->asBuildingObject();

			if (building)
				unloaded = building->unloadContents();
		}

		if (unloaded)
			result += getErrorMessage(argv[0], ERR_SUCCESS);
		else
			result += getErrorMessage(argv[0], ERR_FAIL);
	}

	// ----------------------------------------------------------------------

	else if(isCommand(argv[0], "isOnSolidFloor"))
	{
		NetworkId const oid(Unicode::wideToNarrow(argv[1]));
		Object const * const o = NetworkIdManager::getObjectById(oid);
		if (o)
		{
			CollisionProperty const * const collisionProperty = o->getCollisionProperty();
			if (collisionProperty)
			{
				Footprint const * const footprint = collisionProperty->getFootprint();
				if (footprint)
				{
					result += Unicode::narrowToWide(FormattedString<1024>().sprintf("object (%s) isOnSolidFloor() = %s.\n", oid.getValueString().c_str(), (footprint->isOnSolidFloor() ? "TRUE" : "FALSE")));
					result += Unicode::narrowToWide(FormattedString<1024>().sprintf("object (%s) isStandingOnFloor() = %s.\n", oid.getValueString().c_str(), (CollisionWorld::getFloorStandingOn(*o) ? "TRUE" : "FALSE")));

					bool success;
					float height;

					success = footprint->getTerrainHeight(height);
					if (success)
						result += Unicode::narrowToWide(FormattedString<1024>().sprintf("object (%s) getTerrainHeight() = %.10g.\n", oid.getValueString().c_str(), height));
					else
						result += Unicode::narrowToWide(FormattedString<1024>().sprintf("object (%s) getTerrainHeight() = NONE.\n", oid.getValueString().c_str()));

					success = footprint->getFloorHeight(height);
					if (success)
						result += Unicode::narrowToWide(FormattedString<1024>().sprintf("object (%s) getFloorHeight() = %.10g.\n", oid.getValueString().c_str(), height));
					else
						result += Unicode::narrowToWide(FormattedString<1024>().sprintf("object (%s) getFloorHeight() = NONE.\n", oid.getValueString().c_str()));

					success = footprint->getGroundHeight(height);
					if (success)
						result += Unicode::narrowToWide(FormattedString<1024>().sprintf("object (%s) getGroundHeight() = %.10g.\n", oid.getValueString().c_str(), height));
					else
						result += Unicode::narrowToWide(FormattedString<1024>().sprintf("object (%s) getGroundHeight() = NONE.\n", oid.getValueString().c_str()));
				}
				else
				{
					result += Unicode::narrowToWide(FormattedString<1024>().sprintf("object (%s) getFootprint() returns nullptr.\n", oid.getValueString().c_str()));
				}
			}
			else
			{
				result += Unicode::narrowToWide(FormattedString<1024>().sprintf("object (%s) getCollisionProperty() returns nullptr.\n", oid.getValueString().c_str()));
			}
		}
		else
		{
			result += Unicode::narrowToWide(FormattedString<1024>().sprintf("object (%s) not found.\n", oid.getValueString().c_str()));
		}
	}

	// ----------------------------------------------------------------------

	else
	{
		result += getErrorMessage(argv[0], ERR_NO_HANDLER);
	}

	return true;
}

// ---------------------------------------------------------------------

bool ConsoleCommandParserObject::moveObject (const NetworkId & oid, const Vector & position_w, const NetworkId & targetContainer, const Vector & position_p) const
{
	if (oid < ConfigServerGame::getMaxGoldNetworkId())
		return false;
	
	ServerObject *o = ServerWorld::findObjectByNetworkId(oid);

	if (o)
	{
		// no moving cells
		if (o->asCellObject())
			return false;
		// no moving buildings or ships to containers
		if (targetContainer != NetworkId::cms_invalid && (o->asBuildingObject() || o->asShipObject()))
			return false;
		// no moving contained ships
		if (o->asShipObject() && ContainerInterface::getContainedByObject(*o))
			return false;
		// no moving creatures in space
		if (ServerWorld::isSpaceScene() && o->asCreatureObject())
			return false;
		// no moving objects that aren't ships into the world in space
		if (ServerWorld::isSpaceScene() && targetContainer==NetworkId::cms_invalid && !o->asShipObject())
			return false;

		o->teleportObject(position_w, targetContainer, "", position_p, "");
		return true;
	}
	return false;
}

//----------------------------------------------------------------------

bool ConsoleCommandParserObject::rotateObject (const NetworkId & oid, real yaw, real pitch, real roll) const
{
	bool retval = false;
	Object *o = ServerWorld::findObjectByNetworkId(oid);

	if (o)
	{
		Transform t(o->getTransform_o2p());
		t.yaw_l(yaw);
		t.pitch_l(pitch);
		t.roll_l(roll);
		o->setTransform_o2p(t);
		retval = true;
	}
	return retval;
	
}

// ======================================================================

