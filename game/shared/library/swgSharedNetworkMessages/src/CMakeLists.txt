
set(SHARED_SOURCES
	shared/combat/CombatActionCompleteMessage.cpp
	shared/combat/CombatActionCompleteMessage.h
	shared/combat/MessageQueueCombatAction.cpp
	shared/combat/MessageQueueCombatAction.h
	shared/combat/MessageQueueCombatDamage.cpp
	shared/combat/MessageQueueCombatDamage.h

	shared/consent/ConsentRequestMessage.cpp
	shared/consent/ConsentRequestMessage.h

	shared/core/FirstSwgSharedNetworkMessages.h
	shared/core/SetupSwgSharedNetworkMessages.cpp
	shared/core/SetupSwgSharedNetworkMessages.h

	shared/permissionList/PermissionListCreateMessage.cpp
	shared/permissionList/PermissionListCreateMessage.h
	shared/permissionList/PermissionListModifyMessage.cpp
	shared/permissionList/PermissionListModifyMessage.h

	shared/survey/ResourceListForSurveyMessage.cpp
	shared/survey/ResourceListForSurveyMessage.h
	shared/survey/SurveyMessage.cpp
	shared/survey/SurveyMessage.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/FirstSwgSharedNetworkMessages.cpp
	)
else()
	set(PLATFORM_SOURCES "")
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/shared
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedDebug/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFile/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundation/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundationTypes/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedGame/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedMath/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedMathArchive/include/public
	
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedMessageDispatch/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedNetworkMessages/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedObject/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedRandom/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedUtility/include/public
	${SWG_GAME_SOURCE_DIR}/shared/library/swgSharedUtility/include/public
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/archive/include
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/localization/include
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/localizationArchive/include/public
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/unicode/include
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/unicodeArchive/include/public
)

add_library(swgSharedNetworkMessages STATIC
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)

target_link_libraries(swgSharedNetworkMessages
	sharedGame
	sharedObject
	swgSharedUtility
)
