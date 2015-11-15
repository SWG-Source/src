// ======================================================================
//
// Packager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
// Edit Packager_cpp.template.  Do not edit Packager.cpp
//
// To change the contents of the addMemebersToPackage functions,
// edit package_data.txt.
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/TangibleObject.h"

#include "serverGame/BattlefieldMarkerObject.h"
#include "serverGame/BuildingObject.h"
#include "serverGame/CellObject.h"
#include "serverGame/CityObject.h"
#include "serverGame/CommandQueue.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/DraftSchematicObject.h"
#include "serverGame/FactoryObject.h"
#include "serverGame/GroupObject.h"
#include "serverGame/GuildObject.h"
#include "serverGame/HarvesterInstallationObject.h"
#include "serverGame/InstallationObject.h"
#include "serverGame/IntangibleObject.h"
#include "serverGame/ManufactureSchematicObject.h"
#include "serverGame/ManufactureInstallationObject.h"
#include "serverGame/MissionObject.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/PlayerQuestObject.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/ResourceContainerObject.h"
#include "serverGame/ResourcePoolObject.h"
#include "serverGame/ResourceTypeObject.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ShipObject.h"
#include "serverGame/StaticObject.h"
#include "serverGame/TangibleObject.h"
#include "serverGame/UniverseObject.h"
#include "serverGame/VehicleObject.h"
#include "serverGame/WeaponObject.h"
#include "serverScript/GameScriptObject.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedObject/ContainedByProperty.h"
#include "sharedObject/SlottedContainmentProperty.h"


//!!!BEGIN GENERATED PACKAGEADD
/*
 * Generated function.  Do not edit.
 */
void BattlefieldMarkerObject::addMembersToPackages()
{
	addServerVariable    (m_regionName);
	addServerVariable    (m_battlefieldParticipants);
}

/*
 * Generated function.  Do not edit.
 */
void BuildingObject::addMembersToPackages()
{
	addServerVariable    (m_allowed);
	addServerVariable    (m_banned);
	addServerVariable    (m_isPublic);
	addServerVariable    (m_maintenanceCost);
	addServerVariable    (m_timeLastChecked);
	addServerVariable    (m_cityId);
	addServerVariable_np (m_contentsLoaded);
}

/*
 * Generated function.  Do not edit.
 */
void CellObject::addMembersToPackages()
{
	addServerVariable    (m_allowed);
	addServerVariable    (m_banned);
	addSharedVariable    (m_isPublic);
	addSharedVariable    (m_cellNumber);
	addSharedVariable_np (m_cellLabel);
	addSharedVariable_np (m_labelLocationOffset);
}

/*
 * Generated function.  Do not edit.
 */
void CityObject::addMembersToPackages()
{
	addServerVariable    (m_cities);
	addServerVariable    (m_citizens);
	addServerVariable    (m_structures);
	addServerVariable_np (m_citiesInfo);
	addServerVariable_np (m_citizensInfo);
	addServerVariable_np (m_structuresInfo);
	addServerVariable_np (m_citizenToCityId);
	addServerVariable_np (m_pgcRatingInfo);
	addServerVariable_np (m_pgcRatingChroniclerId);
	addServerVariable_np (m_gcwRegionDefenderCities);
	addServerVariable_np (m_gcwRegionDefenderCitiesCount);
	addServerVariable_np (m_gcwRegionDefenderCitiesVersion);
}

/*
 * Generated function.  Do not edit.
 */
void CreatureObject::addMembersToPackages()
{
	addServerVariable    (m_attributes);
	addServerVariable    (m_baseRunSpeed);
	addServerVariable    (m_baseWalkSpeed);
	addServerVariable    (m_persistedBuffs);
	addServerVariable    (m_wsX);
	addServerVariable    (m_wsY);
	addServerVariable    (m_wsZ);
	addSharedVariable    (m_posture);
	addSharedVariable    (m_rank);
	addSharedVariable    (m_masterId);
	addSharedVariable    (m_scaleFactor);
	addSharedVariable    (m_shockWounds);
	addSharedVariable    (m_states);
	addAuthClientServerVariable    (m_maxAttributes);
	addAuthClientServerVariable    (m_skills);
	addServerVariable_np (m_notifyRegions);
	addServerVariable_np (m_missionCriticalObjectSet);
	addServerVariable_np (m_attributeModList);
	addServerVariable_np (m_baseSlopeModPercent);
	addServerVariable_np (m_cachedCurrentAttributeModValues);
	addServerVariable_np (m_cachedMaxAttributeModValues);
	getCommandQueue()->addToPackage(m_serverPackage_np);
	addServerVariable_np (m_cover);
	addServerVariable_np (m_currentAttitude);
	addServerVariable_np (m_isStatic);
	addServerVariable_np (m_lastBehavior);
	addServerVariable_np (m_lastMonitorReportPosition);
	addServerVariable_np (m_locomotion);
	addServerVariable_np (m_maxMentalStates);
	addServerVariable_np (m_mentalStateDecays);
	addServerVariable_np (m_mentalStatesToward);
	addServerVariable_np (m_monitoredCreatureMovements);
	addServerVariable_np (m_performanceWatchTarget);
	addServerVariable_np (m_stopWalkRun);
	addServerVariable_np (m_timeToUpdateGuildWarPvpStatus);
	addServerVariable_np (m_guildWarEnabled);
	addServerVariable_np (m_militiaOfCityId);
	addServerVariable_np (m_locatedInCityId);
	addServerVariable_np (m_invulnerabilityTimer);
	addServerVariable_np (m_allowSARegen);
	addServerVariable_np (m_inviterForPendingGroup);
	addServerVariable_np (m_timedMod);
	addServerVariable_np (m_timedModDuration);
	addServerVariable_np (m_timedModUpdateTime);
	addSharedVariable_np (m_level);
	addSharedVariable_np (m_levelHealthGranted);
	addSharedVariable_np (m_animatingSkillData);
	addSharedVariable_np (m_animationMood);
	addSharedVariable_np (m_currentWeapon);
	addSharedVariable_np (m_group);
	addSharedVariable_np (m_groupInviter);
	addSharedVariable_np (m_guildId);
	addSharedVariable_np (m_lookAtTarget);
	addSharedVariable_np (m_intendedTarget);
	addSharedVariable_np (m_mood);
	addSharedVariable_np (m_performanceStartTime);
	addSharedVariable_np (m_performanceType);
	addSharedVariable_np (m_totalAttributes);
	addSharedVariable_np (m_totalMaxAttributes);
	addSharedVariable_np (m_wearableData);
	addSharedVariable_np (m_alternateAppearanceSharedObjectTemplateName);
	addSharedVariable_np (m_coverVisibility);
	addSharedVariable_np (m_buffs);
	addSharedVariable_np (m_clientUsesAnimationLocomotion);
	addSharedVariable_np (m_difficulty);
	addSharedVariable_np (m_hologramType);
	addSharedVariable_np (m_visibleOnMapAndRadar);
	addSharedVariable_np (m_isBeast);
	addSharedVariable_np (m_forceShowHam);
	addSharedVariable_np (m_wearableAppearanceData);
	addSharedVariable_np (m_decoyOrigin);
	addAuthClientServerVariable_np (m_accelPercent);
	addAuthClientServerVariable_np (m_accelScale);
	addAuthClientServerVariable_np (m_attribBonus);
	addAuthClientServerVariable_np (m_modMap);
	addAuthClientServerVariable_np (m_movementPercent);
	addAuthClientServerVariable_np (m_movementScale);
	addAuthClientServerVariable_np (m_performanceListenTarget);
	addAuthClientServerVariable_np (m_runSpeed);
	addAuthClientServerVariable_np (m_slopeModAngle);
	addAuthClientServerVariable_np (m_slopeModPercent);
	addAuthClientServerVariable_np (m_turnScale);
	addAuthClientServerVariable_np (m_walkSpeed);
	addAuthClientServerVariable_np (m_waterModPercent);
	addAuthClientServerVariable_np (m_groupMissionCriticalObjectSet);
	addAuthClientServerVariable_np (m_commands);
	addAuthClientServerVariable_np (m_totalLevelXp);
}

/*
 * Generated function.  Do not edit.
 */
void FactoryObject::addMembersToPackages()
{
	addServerVariable_np (m_craftingCount);
	addServerVariable_np (m_craftingSchematic);
	addServerVariable_np (m_attributes);
}

/*
 * Generated function.  Do not edit.
 */
void GroupObject::addMembersToPackages()
{
	addServerVariable_np (m_groupPOBShipAndOwners);
	addServerVariable_np (m_groupMemberLevels);
	addServerVariable_np (m_groupMemberProfessions);
	addServerVariable_np (m_allMembers);
	addServerVariable_np (m_nonPCMembers);
	addSharedVariable_np (m_groupMembers);
	addSharedVariable_np (m_groupShipFormationMembers);
	addSharedVariable_np (m_groupName);
	addSharedVariable_np (m_groupLevel);
	addSharedVariable_np (m_formationNameCrc);
	addSharedVariable_np (m_lootMaster);
	addSharedVariable_np (m_lootRule);
	addSharedVariable_np (m_groupPickupTimer);
	addSharedVariable_np (m_groupPickupLocation);
}

/*
 * Generated function.  Do not edit.
 */
void GuildObject::addMembersToPackages()
{
	addServerVariable    (m_names);
	addServerVariable    (m_leaders);
	addServerVariable    (m_members);
	addServerVariable    (m_enemies);
	addSharedVariable    (m_abbrevs);
	addServerVariable_np (m_guildsInfo);
	addServerVariable_np (m_membersInfo);
	addServerVariable_np (m_fullMembers);
	addServerVariable_np (m_sponsoredMembers);
	addServerVariable_np (m_guildLeaders);
	addServerVariable_np (m_gcwRegionDefenderBonus);
	addServerVariable_np (m_gcwImperialScorePercentileHistoryCountThisGalaxy);
	addServerVariable_np (m_gcwGroupImperialScorePercentileHistoryCountThisGalaxy);
	addServerVariable_np (m_gcwGroupCategoryImperialScoreRawThisGalaxy);
	addServerVariable_np (m_gcwGroupImperialScoreRawThisGalaxy);
	addServerVariable_np (m_gcwImperialScoreOtherGalaxies);
	addServerVariable_np (m_gcwRebelScoreOtherGalaxies);
	addServerVariable_np (m_gcwRegionDefenderGuilds);
	addServerVariable_np (m_gcwRegionDefenderGuildsCount);
	addServerVariable_np (m_gcwRegionDefenderGuildsVersion);
	addSharedVariable_np (m_gcwImperialScorePercentileThisGalaxy);
	addSharedVariable_np (m_gcwGroupImperialScorePercentileThisGalaxy);
	addSharedVariable_np (m_gcwImperialScorePercentileHistoryThisGalaxy);
	addSharedVariable_np (m_gcwGroupImperialScorePercentileHistoryThisGalaxy);
	addSharedVariable_np (m_gcwImperialScorePercentileOtherGalaxies);
	addSharedVariable_np (m_gcwGroupImperialScorePercentileOtherGalaxies);
}

/*
 * Generated function.  Do not edit.
 */
void HarvesterInstallationObject::addMembersToPackages()
{
	addServerVariable    (m_installedEfficiency);
	addServerVariable    (m_resourceType);
	addServerVariable    (m_maxExtractionRate);
	addServerVariable    (m_currentExtractionRate);
	addServerVariable    (m_maxHopperAmount);
	addServerVariable    (m_hopperResource);
	addServerVariable    (m_hopperAmount);
}

/*
 * Generated function.  Do not edit.
 */
void InstallationObject::addMembersToPackages()
{
	addServerVariable    (m_installationType);
	addServerVariable    (m_tickCount);
	addServerVariable    (m_activateStartTime);
	addSharedVariable    (m_activated);
	addSharedVariable    (m_power);
	addSharedVariable    (m_powerRate);
}

/*
 * Generated function.  Do not edit.
 */
void IntangibleObject::addMembersToPackages()
{
	addSharedVariable    (m_count);
	addServerVariable_np (m_crcs);
	addServerVariable_np (m_positions);
	addServerVariable_np (m_headings);
	addServerVariable_np (m_scripts);
	addServerVariable_np (m_player);
	addServerVariable_np (m_objects);
	addServerVariable_np (m_center);
	addServerVariable_np (m_radius);
	addServerVariable_np (m_creator);
	addServerVariable_np (m_theaterName);
}

/*
 * Generated function.  Do not edit.
 */
void ManufactureInstallationObject::addMembersToPackages()
{
}

/*
 * Generated function.  Do not edit.
 */
void ManufactureSchematicObject::addMembersToPackages()
{
	addServerVariable    (m_draftSchematic);
	addServerVariable    (m_creatorId);
	addServerVariable    (m_creatorName);
	addSharedVariable    (m_attributes);
	addSharedVariable    (m_itemsPerContainer);
	addSharedVariable    (m_manufactureTime);
	addServerVariable_np (m_factories);
	addServerVariable_np (m_resourceMaxAttributes);
	addSharedVariable_np (m_appearanceData);
	addSharedVariable_np (m_customAppearance);
	addSharedVariable_np (m_draftSchematicSharedTemplate);
	addSharedVariable_np (m_isCrafting);
	addSharedVariable_np (m_schematicChangedSignal);
}

/*
 * Generated function.  Do not edit.
 */
void MissionObject::addMembersToPackages()
{
	addServerVariable    (m_rootScriptName);
	addServerVariable    (m_missionHolderId);
	addSharedVariable    (m_difficulty);
	addSharedVariable    (m_endLocation);
	addSharedVariable    (m_missionCreator);
	addSharedVariable    (m_reward);
	addSharedVariable    (m_startLocation);
	addSharedVariable    (m_targetAppearance);
	addSharedVariable    (m_description);
	addSharedVariable    (m_title);
	addSharedVariable    (m_status);
	addSharedVariable    (m_missionType);
	addSharedVariable    (m_targetName);
	addSharedVariable    (m_waypoint);
}

/*
 * Generated function.  Do not edit.
 */
void PlanetObject::addMembersToPackages()
{
	addServerVariable    (m_planetName);
	addServerVariable_np (m_travelPointList);
	addServerVariable_np (m_weatherIndex);
	addServerVariable_np (m_windVelocityX);
	addServerVariable_np (m_windVelocityY);
	addServerVariable_np (m_windVelocityZ);
	addServerVariable_np (m_mapLocationMapStatic);
	addServerVariable_np (m_mapLocationMapDynamic);
	addServerVariable_np (m_mapLocationMapPersist);
	addServerVariable_np (m_mapLocationVersionStatic);
	addServerVariable_np (m_mapLocationVersionDynamic);
	addServerVariable_np (m_mapLocationVersionPersist);
	addServerVariable_np (m_collectionServerFirst);
	addServerVariable_np (m_collectionServerFirstUpdateNumber);
	addServerVariable_np (m_connectedCharacterLfgData);
	addServerVariable_np (m_connectedCharacterLfgDataFactionalPresence);
	addServerVariable_np (m_connectedCharacterLfgDataFactionalPresenceGrid);
	addServerVariable_np (m_connectedCharacterBiographyData);
	addServerVariable_np (m_currentEvents);
	addServerVariable_np (m_gcwImperialScore);
	addServerVariable_np (m_gcwRebelScore);
}

/*
 * Generated function.  Do not edit.
 */
void PlayerObject::addMembersToPackages()
{
	addServerVariable    (m_stationId);
	addServerVariable    (m_houseId);
	addServerVariable    (m_accountNumLots);
	addServerVariable    (m_accountMaxLotsAdjustment);
	addServerVariable    (m_accountIsOutcast);
	addServerVariable    (m_accountCheaterLevel);
	addServerVariable    (m_forceRegenRate);
	addServerVariable    (m_currentGcwRating);
	addServerVariable    (m_maxGcwImperialRating);
	addServerVariable    (m_maxGcwRebelRating);
	addServerVariable    (m_nextGcwRatingCalcTime);
	addSharedVariable    (m_matchMakingCharacterProfileId);
	addSharedVariable    (m_matchMakingPersonalProfileId);
	addSharedVariable    (m_skillTitle);
	addSharedVariable    (m_bornDate);
	addSharedVariable    (m_playedTime);
	addSharedVariable    (m_roleIconChoice);
	addSharedVariable    (m_skillTemplate);
	addSharedVariable    (m_currentGcwPoints);
	addSharedVariable    (m_currentPvpKills);
	addSharedVariable    (m_lifetimeGcwPoints);
	addSharedVariable    (m_lifetimePvpKills);
	addSharedVariable    (m_collections);
	addSharedVariable    (m_collections2);
	addSharedVariable    (m_showBackpack);
	addSharedVariable    (m_showHelmet);
	addServerVariable_np (m_craftingTool);
	addServerVariable_np (m_forceRegenValue);
	addServerVariable_np (m_theaterDatatable);
	addServerVariable_np (m_theaterPosition);
	addServerVariable_np (m_theaterScene);
	addServerVariable_np (m_theaterScript);
	addServerVariable_np (m_theaterNumObjects);
	addServerVariable_np (m_theaterRadius);
	addServerVariable_np (m_theaterCreator);
	addServerVariable_np (m_theaterName);
	addServerVariable_np (m_theaterId);
	addServerVariable_np (m_theaterLocationType);
	addServerVariable_np (m_sessionStartPlayTime);
	addServerVariable_np (m_sessionLastActiveTime);
	addServerVariable_np (m_sessionActivePlayTimeDuration);
	addServerVariable_np (m_aggroImmuneStartTime);
	addServerVariable_np (m_aggroImmuneDuration);
	addServerVariable_np (m_isFromLogin);
	addServerVariable_np (m_sessionActivity);
	addServerVariable_np (m_chatSpamSpatialNumCharacters);
	addServerVariable_np (m_chatSpamNonSpatialNumCharacters);
	addServerVariable_np (m_chatSpamTimeEndInterval);
	addServerVariable_np (m_chatSpamNextTimeToSyncWithChatServer);
	addServerVariable_np (m_currentGcwRegion);
	addSharedVariable_np (m_privledgedTitle);
	addSharedVariable_np (m_currentGcwRank);
	addSharedVariable_np (m_currentGcwRankProgress);
	addSharedVariable_np (m_maxGcwImperialRank);
	addSharedVariable_np (m_maxGcwRebelRank);
	addSharedVariable_np (m_gcwRatingActualCalcTime);
	addSharedVariable_np (m_citizenshipCity);
	addSharedVariable_np (m_citizenshipType);
	addSharedVariable_np (m_cityGcwDefenderRegion);
	addSharedVariable_np (m_guildGcwDefenderRegion);
	addSharedVariable_np (m_squelchedById);
	addSharedVariable_np (m_squelchedByName);
	addSharedVariable_np (m_squelchExpireTime);
	addSharedVariable_np (m_environmentFlags);
	addSharedVariable_np (m_defaultAttackOverride);
	addFirstParentAuthClientServerVariable    (m_experiencePoints);
	addFirstParentAuthClientServerVariable    (m_waypoints);
	addFirstParentAuthClientServerVariable    (m_forcePower);
	addFirstParentAuthClientServerVariable    (m_maxForcePower);
	addFirstParentAuthClientServerVariable    (m_completedQuests);
	addFirstParentAuthClientServerVariable    (m_activeQuests);
	addFirstParentAuthClientServerVariable    (m_currentQuest);
	addFirstParentAuthClientServerVariable    (m_quests);
	addFirstParentAuthClientServerVariable    (m_workingSkill);
	addFirstParentAuthClientServerVariable_np (m_craftingLevel);
	addFirstParentAuthClientServerVariable_np (m_craftingStage);
	addFirstParentAuthClientServerVariable_np (m_craftingStation);
	addFirstParentAuthClientServerVariable_np (m_draftSchematics);
	addFirstParentAuthClientServerVariable_np (m_craftingComponentBioLink);
	addFirstParentAuthClientServerVariable_np (m_experimentPoints);
	addFirstParentAuthClientServerVariable_np (m_expModified);
	addFirstParentAuthClientServerVariable_np (m_friendList);
	addFirstParentAuthClientServerVariable_np (m_ignoreList);
	addFirstParentAuthClientServerVariable_np (m_spokenLanguage);
	addFirstParentAuthClientServerVariable_np (m_food);
	addFirstParentAuthClientServerVariable_np (m_maxFood);
	addFirstParentAuthClientServerVariable_np (m_drink);
	addFirstParentAuthClientServerVariable_np (m_maxDrink);
	addFirstParentAuthClientServerVariable_np (m_meds);
	addFirstParentAuthClientServerVariable_np (m_maxMeds);
	addFirstParentAuthClientServerVariable_np (m_groupWaypoints);
	addFirstParentAuthClientServerVariable_np (m_playerHateList);
	addFirstParentAuthClientServerVariable_np (m_killMeter);
	addFirstParentAuthClientServerVariable_np (m_accountNumLotsOverLimitSpam);
	addFirstParentAuthClientServerVariable_np (m_petId);
	addFirstParentAuthClientServerVariable_np (m_petCommandList);
	addFirstParentAuthClientServerVariable_np (m_petToggledCommands);
	addFirstParentAuthClientServerVariable_np (m_guildRank);
	addFirstParentAuthClientServerVariable_np (m_citizenRank);
	addFirstParentAuthClientServerVariable_np (m_galacticReserveDeposit);
	addFirstParentAuthClientServerVariable_np (m_pgcRatingCount);
	addFirstParentAuthClientServerVariable_np (m_pgcRatingTotal);
	addFirstParentAuthClientServerVariable_np (m_pgcLastRatingTime);
}

/*
 * Generated function.  Do not edit.
 */
void PlayerQuestObject::addMembersToPackages()
{
	addSharedVariable    (m_title);
	addSharedVariable    (m_description);
	addSharedVariable    (m_creator);
	addSharedVariable    (m_totalTasks);
	addSharedVariable    (m_difficulty);
	addSharedVariable    (m_taskTitle1);
	addSharedVariable    (m_taskDescription1);
	addSharedVariable    (m_taskTitle2);
	addSharedVariable    (m_taskDescription2);
	addSharedVariable    (m_taskTitle3);
	addSharedVariable    (m_taskDescription3);
	addSharedVariable    (m_taskTitle4);
	addSharedVariable    (m_taskDescription4);
	addSharedVariable    (m_taskTitle5);
	addSharedVariable    (m_taskDescription5);
	addSharedVariable    (m_taskTitle6);
	addSharedVariable    (m_taskDescription6);
	addSharedVariable    (m_taskTitle7);
	addSharedVariable    (m_taskDescription7);
	addSharedVariable    (m_taskTitle8);
	addSharedVariable    (m_taskDescription8);
	addSharedVariable    (m_taskTitle9);
	addSharedVariable    (m_taskDescription9);
	addSharedVariable    (m_taskTitle10);
	addSharedVariable    (m_taskDescription10);
	addSharedVariable    (m_taskTitle11);
	addSharedVariable    (m_taskDescription11);
	addSharedVariable    (m_taskTitle12);
	addSharedVariable    (m_taskDescription12);
	addSharedVariable_np (m_tasks);
	addSharedVariable_np (m_taskCounters);
	addSharedVariable_np (m_taskStatus);
	addSharedVariable_np (m_waypoints);
	addSharedVariable_np (m_rewards);
	addSharedVariable_np (m_creatorName);
	addSharedVariable_np (m_completed);
	addSharedVariable_np (m_recipe);
}

/*
 * Generated function.  Do not edit.
 */
void ResourceContainerObject::addMembersToPackages()
{
	addServerVariable    (m_source);
	addSharedVariable    (m_quantity);
	addSharedVariable    (m_resourceType);
	addSharedVariable_np (m_maxQuantity);
	addSharedVariable_np (m_parentName);
	addSharedVariable_np (m_resourceName);
	addSharedVariable_np (m_resourceNameId);
}

/*
 * Generated function.  Do not edit.
 */
void ServerObject::addMembersToPackages()
{
	addServerVariable    (m_cacheVersion);
	addServerVariable    (m_loadContents);
	;
	m_objVars.addToPackage(m_serverPackage,m_serverPackage_np);
	addServerVariable    (m_persisted);
	addServerVariable    (m_playerControlled);
	addServerVariable    (m_sceneId);
	m_scriptObject->addToPackage(m_serverPackage);
	addServerVariable    (m_conversionId);
	addServerVariable    (m_staticItemName);
	addServerVariable    (m_staticItemVersion);
	addSharedVariable    (m_complexity);
	addSharedVariable    (m_nameStringId);
	addSharedVariable    (m_objectName);
	addSharedVariable    (m_volume);
	addAuthClientServerVariable    (m_bankBalance);
	addAuthClientServerVariable    (m_cashBalance);
	addServerVariable_np (m_attributesAttained);
	addServerVariable_np (m_attributesInterested);
	addServerVariable_np (m_proxyServerProcessIds);
	addServerVariable_np (m_transformSequence);
	addServerVariable_np (m_triggerVolumeInfo);
	addServerVariable_np (m_contentsLoaded);
	addServerVariable_np (m_contentsRequested);
	addServerVariable_np (m_messageTos);
	addServerVariable_np (m_defaultAlterTime);
	addServerVariable_np (m_observersCount);
	addServerVariable_np (m_includeInBuildout);
	addServerVariable_np (m_broadcastListeners);
	addServerVariable_np (m_broadcastBroadcasters);
	addSharedVariable_np (m_authServerProcessId);
	addSharedVariable_np (m_descriptionStringId);
}

/*
 * Generated function.  Do not edit.
 */
void ShipObject::addMembersToPackages()
{
	addServerVariable    (m_componentCrc);
	addSharedVariable    (m_slideDampener);
	addSharedVariable    (m_currentChassisHitPoints);
	addSharedVariable    (m_maximumChassisHitPoints);
	addSharedVariable    (m_chassisType);
	addSharedVariable    (m_componentArmorHitpointsMaximum);
	addSharedVariable    (m_componentArmorHitpointsCurrent);
	addSharedVariable    (m_componentHitpointsCurrent);
	addSharedVariable    (m_componentHitpointsMaximum);
	addSharedVariable    (m_componentFlags);
	addSharedVariable    (m_shieldHitpointsFrontMaximum);
	addSharedVariable    (m_shieldHitpointsBackMaximum);
	addAuthClientServerVariable    (m_componentEfficiencyGeneral);
	addAuthClientServerVariable    (m_componentEfficiencyEnergy);
	addAuthClientServerVariable    (m_componentEnergyMaintenanceRequirement);
	addAuthClientServerVariable    (m_componentMass);
	addAuthClientServerVariable    (m_componentNames);
	addAuthClientServerVariable    (m_componentCreators);
	addAuthClientServerVariable    (m_weaponDamageMaximum);
	addAuthClientServerVariable    (m_weaponDamageMinimum);
	addAuthClientServerVariable    (m_weaponEffectivenessShields);
	addAuthClientServerVariable    (m_weaponEffectivenessArmor);
	addAuthClientServerVariable    (m_weaponEnergyPerShot);
	addAuthClientServerVariable    (m_weaponRefireRate);
	addAuthClientServerVariable    (m_weaponAmmoCurrent);
	addAuthClientServerVariable    (m_weaponAmmoMaximum);
	addAuthClientServerVariable    (m_weaponAmmoType);
	addAuthClientServerVariable    (m_chassisComponentMassMaximum);
	addAuthClientServerVariable    (m_shieldRechargeRate);
	addAuthClientServerVariable    (m_capacitorEnergyMaximum);
	addAuthClientServerVariable    (m_capacitorEnergyRechargeRate);
	addAuthClientServerVariable    (m_engineAccelerationRate);
	addAuthClientServerVariable    (m_engineDecelerationRate);
	addAuthClientServerVariable    (m_enginePitchAccelerationRate);
	addAuthClientServerVariable    (m_engineYawAccelerationRate);
	addAuthClientServerVariable    (m_engineRollAccelerationRate);
	addAuthClientServerVariable    (m_enginePitchRateMaximum);
	addAuthClientServerVariable    (m_engineYawRateMaximum);
	addAuthClientServerVariable    (m_engineRollRateMaximum);
	addAuthClientServerVariable    (m_engineSpeedMaximum);
	addAuthClientServerVariable    (m_reactorEnergyGenerationRate);
	addAuthClientServerVariable    (m_boosterEnergyMaximum);
	addAuthClientServerVariable    (m_boosterEnergyRechargeRate);
	addAuthClientServerVariable    (m_boosterEnergyConsumptionRate);
	addAuthClientServerVariable    (m_boosterAcceleration);
	addAuthClientServerVariable    (m_boosterSpeedMaximum);
	addAuthClientServerVariable    (m_droidInterfaceCommandSpeed);
	addAuthClientServerVariable    (m_installedDroidControlDevice);
	addAuthClientServerVariable    (m_cargoHoldContentsMaximum);
	addAuthClientServerVariable    (m_cargoHoldContentsCurrent);
	addAuthClientServerVariable    (m_cargoHoldContents);
	addServerVariable_np (m_engineSpeedRotationFactorMaximum);
	addServerVariable_np (m_engineSpeedRotationFactorMinimum);
	addServerVariable_np (m_engineSpeedRotationFactorOptimal);
	addSharedVariable_np (m_shipId);
	addSharedVariable_np (m_shipActualAccelerationRate);
	addSharedVariable_np (m_shipActualDecelerationRate);
	addSharedVariable_np (m_shipActualPitchAccelerationRate);
	addSharedVariable_np (m_shipActualYawAccelerationRate);
	addSharedVariable_np (m_shipActualRollAccelerationRate);
	addSharedVariable_np (m_shipActualPitchRateMaximum);
	addSharedVariable_np (m_shipActualYawRateMaximum);
	addSharedVariable_np (m_shipActualRollRateMaximum);
	addSharedVariable_np (m_shipActualSpeedMaximum);
	addSharedVariable_np (m_pilotLookAtTarget);
	addSharedVariable_np (m_pilotLookAtTargetSlot);
	addSharedVariable_np (m_targetableSlotBitfield);
	addSharedVariable_np (m_componentCrcForClient);
	addSharedVariable_np (m_wingName);
	addSharedVariable_np (m_typeName);
	addSharedVariable_np (m_difficulty);
	addSharedVariable_np (m_faction);
	addSharedVariable_np (m_shieldHitpointsFrontCurrent);
	addSharedVariable_np (m_shieldHitpointsBackCurrent);
	addSharedVariable_np (m_guildId);
	addAuthClientServerVariable_np (m_chassisComponentMassCurrent);
	addAuthClientServerVariable_np (m_chassisSpeedMaximumModifier);
	addAuthClientServerVariable_np (m_capacitorEnergyCurrent);
	addAuthClientServerVariable_np (m_boosterEnergyCurrent);
	addAuthClientServerVariable_np (m_weaponEfficiencyRefireRate);
	addAuthClientServerVariable_np (m_cargoHoldContentsResourceTypeInfo);
}

/*
 * Generated function.  Do not edit.
 */
void StaticObject::addMembersToPackages()
{
}

/*
 * Generated function.  Do not edit.
 */
void TangibleObject::addMembersToPackages()
{
	addServerVariable    (m_customAppearance);
	addServerVariable    (m_locationTargets);
	addServerVariable    (m_ownerId);
	addServerVariable    (m_creatorId);
	addServerVariable    (m_sourceDraftSchematic);
	addSharedVariable    (m_pvpFaction);
	addSharedVariable    (m_pvpType);
	addSharedVariable    (m_appearanceData);
	addSharedVariable    (m_components);
	addSharedVariable    (m_condition);
	addSharedVariable    (m_count);
	addSharedVariable    (m_damageTaken);
	addSharedVariable    (m_maxHitPoints);
	addSharedVariable    (m_visible);
	addServerVariable_np (m_pvpEnemies);
	addServerVariable_np (m_pvpMercenaryFaction);
	addServerVariable_np (m_pvpMercenaryType);
	addServerVariable_np (m_pvpFutureType);
	addServerVariable_np (m_hateOverTime);
	addServerVariable_np (m_pvpRegionCrc);
	addServerVariable_np (m_conversations);
	addServerVariable_np (m_hideFromClient);
	addServerVariable_np (m_combatStartTime);
	addServerVariable_np (m_attackableOverride);
	addServerVariable_np (m_passiveReveal);
	addSharedVariable_np (m_inCombat);
	addSharedVariable_np (m_passiveRevealPlayerCharacter);
	addSharedVariable_np (m_mapColorOverride);
	addSharedVariable_np (m_accessList);
	addSharedVariable_np (m_guildAccessList);
	addSharedVariable_np (m_effectsMap);
}

/*
 * Generated function.  Do not edit.
 */
void UniverseObject::addMembersToPackages()
{
}

/*
 * Generated function.  Do not edit.
 */
void VehicleObject::addMembersToPackages()
{
	addServerVariable    (m_bogus);
}

/*
 * Generated function.  Do not edit.
 */
void WeaponObject::addMembersToPackages()
{
	addServerVariable    (m_minDamage);
	addServerVariable    (m_maxDamage);
	addServerVariable    (m_woundChance);
	addServerVariable    (m_attackCost);
	addServerVariable    (m_damageRadius);
	addSharedVariable    (m_attackSpeed);
	addSharedVariable    (m_accuracy);
	addSharedVariable    (m_minRange);
	addSharedVariable    (m_maxRange);
	addSharedVariable    (m_damageType);
	addSharedVariable    (m_elementalType);
	addSharedVariable    (m_elementalValue);
	addServerVariable_np (m_isDefaultWeapon);
	addSharedVariable_np (m_weaponType);
}

//!!!END GENERATED PACKAGEADD
