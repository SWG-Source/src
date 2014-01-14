// ======================================================================
//
// SetupSharedGame.cpp
// Copyright 2002-2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/SetupSharedGame.h"

#include "LocalizationManager.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedGame/AppearanceManager.h"
#include "sharedGame/AssetCustomizationManager.h"
#include "sharedGame/AsteroidGenerationManager.h"
#include "sharedGame/CitizenRankDataTable.h"
#include "sharedGame/ClientCombatManagerSupport.h"
#include "sharedGame/CollectionsDataTable.h"
#include "sharedGame/CombatDataTable.h"
#include "sharedGame/ConfigSharedGame.h"
#include "sharedGame/CustomizationManager.h"
#include "sharedGame/GameLanguageManager.h"
#include "sharedGame/GameScheduler.h"
#include "sharedGame/GroundZoneManager.h"
#include "sharedGame/GuildRankDataTable.h"
#include "sharedGame/HyperspaceManager.h"
#include "sharedGame/LfgDataTable.h"
#include "sharedGame/MoodManager.h"
#include "sharedGame/MountValidScaleRangeTable.h"
#include "sharedGame/NebulaManager.h"
#include "sharedGame/ObjectUsabilityManager.h"
#include "sharedGame/PlayerFormationManager.h"
#include "sharedGame/SharedBuffBuilderManager.h"
#include "sharedGame/SharedBuildoutAreaManager.h"
#include "sharedGame/SharedImageDesignerManager.h"
#include "sharedGame/SharedSaddleManager.h"
#include "sharedGame/ShipChassis.h"
#include "sharedGame/ShipComponentAttachmentManager.h"
#include "sharedGame/ShipComponentDescriptor.h"
#include "sharedGame/ShipComponentWeaponManager.h"
#include "sharedGame/ShipSlotIdManager.h"
#include "sharedGame/ShipTurretManager.h"
#include "sharedGame/SpatialChatManager.h"
#include "sharedGame/TextManager.h"
#include "sharedGame/TravelManager.h"
#include "sharedGame/Waypoint.h"
#include "sharedGame/WearableAppearanceMap.h"

// ======================================================================

namespace SetupSharedGameNamespace
{
	bool ms_installed;

	void remove ();
}

using namespace SetupSharedGameNamespace;

// ======================================================================
// PUBLIC SetupSharedGame::Data
// ======================================================================

SetupSharedGame::Data::Data () :
m_debugBadStringsFunc (0),
m_useGameScheduler    (false),
m_useMountValidScaleRangeTable (false),
m_useWearableAppearanceMap (true),
m_useClientCombatManagerSupport (false)
{
}

// ----------------------------------------------------------------------

void SetupSharedGame::Data::setUseMountValidScaleRangeTable (bool useIt)
{
	m_useMountValidScaleRangeTable = useIt;
}

// ----------------------------------------------------------------------

bool SetupSharedGame::Data::getUseMountValidScaleRangeTable () const
{
	return m_useMountValidScaleRangeTable;
}

// ----------------------------------------------------------------------

void SetupSharedGame::Data::setUseGameScheduler (bool useIt)
{
	m_useGameScheduler = useIt;
}

// ----------------------------------------------------------------------

bool SetupSharedGame::Data::getUseGameScheduler () const
{
	return m_useGameScheduler;
}

// ----------------------------------------------------------------------

void SetupSharedGame::Data::setUseWearableAppearanceMap (bool useIt)
{
	m_useWearableAppearanceMap = useIt;
}

// ----------------------------------------------------------------------

bool SetupSharedGame::Data::getUseWearableAppearanceMap () const
{
	return m_useWearableAppearanceMap;
}

// ----------------------------------------------------------------------

void SetupSharedGame::Data::setUseClientCombatManagerSupport(bool useIt)
{
	m_useClientCombatManagerSupport = useIt;
}

// ----------------------------------------------------------------------

bool SetupSharedGame::Data::getUseClientCombatManagerSupport() const
{
	return m_useClientCombatManagerSupport;
}

// ======================================================================
// STATIC PUBLIC SetupSharedGame
// ======================================================================

void SetupSharedGame::install (const SetupSharedGame::Data& data)
{
	InstallTimer const installTimer("SetupSharedGame::install");

	DEBUG_FATAL (ms_installed, ("SetupSharedGame::install already installed"));
	ms_installed = true;

	ConfigSharedGame::install();

	//-- TODO: add to ExitChain in install functions
	SpatialChatManager::install();
	ExitChain::add(SpatialChatManager::remove, "SpatialChatManager::remove");

	MoodManager::install();
	ExitChain::add(MoodManager::remove, "MoodManager::remove");

	const bool displayBadStringIds   = ConfigSharedGame::getDisplayBadStringIds ();
	const bool debugStringIds        = ConfigSharedGame::getDebugStringIds      ();
	Unicode::NarrowString defaultLocale(ConfigSharedGame::getDefaultLocale ());
	Unicode::UnicodeNarrowStringVector localeVector;
	localeVector.push_back(defaultLocale);

	LocalizationManager::install (new TreeFile::TreeFileFactory, localeVector, debugStringIds, data.m_debugBadStringsFunc, displayBadStringIds);
	ExitChain::add(LocalizationManager::remove, "LocalizationManager::remove");

	TravelManager::install ();

	if (data.getUseGameScheduler ())
		GameScheduler::install ();

	AppearanceManager::install();
	CustomizationManager::install();
	SharedBuffBuilderManager::install();
	SharedImageDesignerManager::install();
	TextManager::install();
	GameLanguageManager::install();
	ShipChassis::install ();
	ShipComponentDescriptor::install ();
	ShipComponentWeaponManager::install ();
	ShipComponentAttachmentManager::install ();
	ShipSlotIdManager::install ();
	ShipTurretManager::install ();
	ObjectUsabilityManager::install();

	AssetCustomizationManager::install ("customization/asset_customization_manager.iff");

	if (data.getUseMountValidScaleRangeTable ())
		MountValidScaleRangeTable::install ("datatables/mount/valid_scale_range.iff");

	if (data.getUseWearableAppearanceMap ())
		WearableAppearanceMap::install ("datatables/appearance/wearable_appearance_map.iff");

	if (data.getUseClientCombatManagerSupport ())
		ClientCombatManagerSupport::install ("combat/combat_manager.iff");

	AsteroidGenerationManager::install();
	NebulaManager::install();
	HyperspaceManager::install();
	PlayerFormationManager::install();
	CollectionsDataTable::install();
	LfgDataTable::install();
	GuildRankDataTable::install();
	CitizenRankDataTable::install();
	SharedBuildoutAreaManager::install();
	GroundZoneManager::install();

	Waypoint::install();

	SharedSaddleManager::install();
	CombatDataTable::install();
	ExitChain::add (SetupSharedGameNamespace::remove, "SetupSharedGameNamespace::remove");
}

// ======================================================================
// STATIC PRIVATE SetupSharedGame
// ======================================================================

void SetupSharedGameNamespace::remove ()
{
	DEBUG_FATAL (!ms_installed, ("SetupSharedGameNamespace::remove not installed"));
	ms_installed = false;
}

// ======================================================================
