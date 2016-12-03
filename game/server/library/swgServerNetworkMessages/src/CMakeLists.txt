
set(SHARED_SOURCES
	shared/combat/MessageQueueCombatDamageList.cpp
	shared/combat/MessageQueueCombatDamageList.h
	shared/combat/MessageQueueDirectDamage.cpp
	shared/combat/MessageQueueDirectDamage.h

	shared/core/FirstSwgServerNetworkMessages.h
	shared/core/SetupSwgServerNetworkMessages.cpp
	shared/core/SetupSwgServerNetworkMessages.h

	shared/jedi/MessageQueueJediData.cpp
	shared/jedi/MessageQueueJediData.h
	shared/jedi/MessageQueueJediLocation.cpp
	shared/jedi/MessageQueueJediLocation.h
	shared/jedi/MessageQueueRequestJediBounty.cpp
	shared/jedi/MessageQueueRequestJediBounty.h

	shared/money/MessageQueueScriptTransferMoney.cpp
	shared/money/MessageQueueScriptTransferMoney.h

	shared/resource/MessageQueueActivateInstallation.cpp
	shared/resource/MessageQueueActivateInstallation.h
	shared/resource/MessageQueueDeactivateInstallation.cpp
	shared/resource/MessageQueueDeactivateInstallation.h
	shared/resource/MessageQueueInstallationHarvest.cpp
	shared/resource/MessageQueueInstallationHarvest.h
	shared/resource/MessageQueueResourceAdd.cpp
	shared/resource/MessageQueueResourceAdd.h
	shared/resource/MessageQueueResourceFinishHarvest.cpp
	shared/resource/MessageQueueResourceFinishHarvest.h
	shared/resource/MessageQueueResourceSetInstalledEfficiency.cpp
	shared/resource/MessageQueueResourceSetInstalledEfficiency.h
	shared/resource/MessageQueueResourceTypeDepleted.cpp
	shared/resource/MessageQueueResourceTypeDepleted.h

	shared/travel/MessageQueuePlanetAddTravelPoint.cpp
	shared/travel/MessageQueuePlanetAddTravelPoint.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/FirstSwgServerNetworkMessages.cpp
	)
else()
	set(PLATFORM_SOURCES "")
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/shared
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedDebug/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundation/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundationTypes/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedGame/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedMath/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedMathArchive/include/public
	
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedMessageDispatch/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedNetworkMessages/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedObject/include/public
	${SWG_ENGINE_SOURCE_DIR}/server/library/serverNetworkMessages/include/public
	${SWG_ENGINE_SOURCE_DIR}/server/library/serverUtility/include/public
	${SWG_GAME_SOURCE_DIR}/shared/library/swgSharedUtility/include/public
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/archive/include
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/localization/include
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/localizationArchive/include/public
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/unicode/include
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/unicodeArchive/include/public
)

add_library(swgServerNetworkMessages STATIC
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)
