if (${CMAKE_BUILD_TYPE} STREQUAL "MINSIZEREL" AND EXISTS "${PROJECT_SOURCE_DIR}/src/linux/SwgDatabaseServer.profdata")
	add_compile_options(-fprofile-instr-use=${PROJECT_SOURCE_DIR}/src/linux/SwgDatabaseServer.profdata)
endif()

set(SHARED_SOURCES
    shared/buffers/AuctionLocationsBuffer.cpp
    shared/buffers/AuctionLocationsBuffer.h
    shared/buffers/BattlefieldParticipantBuffer.cpp
    shared/buffers/BattlefieldParticipantBuffer.h
    shared/buffers/BountyHunterTargetBuffer.cpp
    shared/buffers/BountyHunterTargetBuffer.h
    shared/buffers/CreatureObjectBuffer.cpp
    shared/buffers/CreatureObjectBuffer.h
    shared/buffers/ExperienceBuffer.cpp
    shared/buffers/ExperienceBuffer.h
    shared/buffers/IndexedNetworkTableBuffer.cpp
    shared/buffers/IndexedNetworkTableBuffer.h
    shared/buffers/LocationBuffer.cpp
    shared/buffers/LocationBuffer.h
    shared/buffers/ManufactureSchematicAttributeBuffer.cpp
    shared/buffers/ManufactureSchematicAttributeBuffer.h
    shared/buffers/MarketAuctionBidsBuffer.cpp
    shared/buffers/MarketAuctionBidsBuffer.h
    shared/buffers/MarketAuctionsBuffer.cpp
    shared/buffers/MarketAuctionsBuffer.h
    shared/buffers/MessageBuffer.cpp
    shared/buffers/MessageBuffer.h
    shared/buffers/ObjectTableBuffer.cpp
    shared/buffers/ObjectTableBuffer.h
    shared/buffers/ObjvarBuffer.cpp
    shared/buffers/ObjvarBuffer.h
    shared/buffers/PropertyListBuffer.cpp
    shared/buffers/PropertyListBuffer.h
    shared/buffers/ResourceTypeBuffer.cpp
    shared/buffers/ResourceTypeBuffer.h
    shared/buffers/ScriptBuffer.cpp
    shared/buffers/ScriptBuffer.h
    shared/buffers/WaypointBuffer.cpp
    shared/buffers/WaypointBuffer.h

    shared/cleanup/DataCleanupManager.cpp
    shared/cleanup/DataCleanupManager.h
    shared/cleanup/TaskConsolidateExperience.cpp
    shared/cleanup/TaskConsolidateExperience.h
    shared/cleanup/TaskDailyCleanup.cpp
    shared/cleanup/TaskDailyCleanup.h
    shared/cleanup/TaskAnySQL.cpp
    shared/cleanup/TaskAnySQL.h
    shared/cleanup/TaskObjvarNameCleanup.cpp
    shared/cleanup/TaskObjvarNameCleanup.h
    shared/cleanup/TaskOrphanedObjectCleanup.cpp
    shared/cleanup/TaskOrphanedObjectCleanup.h
    shared/cleanup/TaskMarketAttributesCleanup.cpp
    shared/cleanup/TaskMarketAttributesCleanup.h
    shared/cleanup/TaskMessagesCleanup.cpp
    shared/cleanup/TaskMessagesCleanup.h
    shared/cleanup/TaskBrokenObjectCleanup.cpp
    shared/cleanup/TaskBrokenObjectCleanup.h
    shared/cleanup/TaskVendorObjectCleanup.cpp
    shared/cleanup/TaskVendorObjectCleanup.h
    shared/cleanup/TaskFixBadCells.cpp
    shared/cleanup/TaskFixBadCells.h
    shared/cleanup/TaskObjectTemplateListUpdater.cpp
    shared/cleanup/TaskObjectTemplateListUpdater.h

    shared/core/CMLoader.cpp
    shared/core/CMLoader.h
    shared/core/FirstSwgDatabaseServer.h
    shared/core/MiscPack.cpp
    shared/core/ObjvarNameManager.cpp
    shared/core/ObjvarNameManager.h
    shared/core/CommoditiesSnapshot.cpp
    shared/core/CommoditiesSnapshot.h
    shared/core/SwgDatabaseServer.cpp
    shared/core/SwgDatabaseServer.h
    shared/core/SwgLoader.cpp
    shared/core/SwgLoader.h
    shared/core/SwgLoaderSnapshotGroup.cpp
    shared/core/SwgLoaderSnapshotGroup.h
    shared/core/SwgPersister.cpp
    shared/core/SwgPersister.h
    shared/core/SwgSnapshot.cpp
    shared/core/SwgSnapshot.h

    shared/persistable/PersistableWaypoint.cpp
    shared/persistable/PersistableWaypoint.h

    shared/queries/BattlefieldParticipantQuery.cpp
    shared/queries/BattlefieldParticipantQuery.h
    shared/queries/BountyHunterTargetQuery.cpp
    shared/queries/BountyHunterTargetQuery.h
    shared/queries/CommoditiesQuery.cpp
    shared/queries/CommoditiesQuery.h
    shared/queries/DeleteDemandLoadedContainerQuery.cpp
    shared/queries/DeleteDemandLoadedContainerQuery.h
    shared/queries/ExperienceQuery.cpp
    shared/queries/ExperienceQuery.h
    shared/queries/LocationQuery.cpp
    shared/queries/LocationQuery.h
    shared/queries/ManufactureSchematicAttributeQuery.cpp
    shared/queries/ManufactureSchematicAttributeQuery.h
    shared/queries/MessageQuery.cpp
    shared/queries/MessageQuery.h
    shared/queries/ObjectVariableQueries.cpp
    shared/queries/ObjectVariableQueries.h
    shared/queries/ObjectsTableQuery.cpp
    shared/queries/ObjectsTableQuery.h
    shared/queries/OfflineMoneyCustomPersistStep.cpp
    shared/queries/OfflineMoneyCustomPersistStep.h
    shared/queries/PropertyListQuery.cpp
    shared/queries/PropertyListQuery.h
    shared/queries/ResourceTypeQuery.cpp
    shared/queries/ResourceTypeQuery.h
    shared/queries/SkillQuery.cpp
    shared/queries/SkillQuery.h
    shared/queries/WaypointQuery.cpp
    shared/queries/WaypointQuery.h

    shared/tasks/TaskGetAuctionAttributeList.cpp
    shared/tasks/TaskGetAuctionAttributeList.h
    shared/tasks/TaskGetAuctionList.cpp
    shared/tasks/TaskGetAuctionList.h
    shared/tasks/TaskGetBidList.cpp
    shared/tasks/TaskGetBidList.h
    shared/tasks/TaskGetCharacters.cpp
    shared/tasks/TaskGetCharacters.h
    shared/tasks/TaskGetLocationList.cpp
    shared/tasks/TaskGetLocationList.h
    shared/tasks/TaskLoadObjvarNames.cpp
    shared/tasks/TaskLoadObjvarNames.h
    shared/tasks/TaskLocateStructure.cpp
    shared/tasks/TaskLocateStructure.h
    shared/tasks/TaskMoveToPlayer.cpp
    shared/tasks/TaskMoveToPlayer.h
    shared/tasks/TaskRestoreCharacter.cpp
    shared/tasks/TaskRestoreCharacter.h
    shared/tasks/TaskRestoreHouse.cpp
    shared/tasks/TaskRestoreHouse.h
    shared/tasks/TaskSaveObjvarNames.cpp
    shared/tasks/TaskSaveObjvarNames.h
    shared/tasks/TaskUndeleteItem.cpp
    shared/tasks/TaskUndeleteItem.h
    shared/tasks/TaskVerifyCharacter.cpp
    shared/tasks/TaskVerifyCharacter.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/FirstSwgDatabaseServer.cpp
		win32/WinMain.cpp
	)
else()
	set(PLATFORM_SOURCES
		linux/main.cpp
	)
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/buffers
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/core
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/generated
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedDatabaseInterface/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedDebug/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFile/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundation/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundationTypes/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedGame/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedLog/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedMath/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedMathArchive/include/public
    
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedMessageDispatch/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedNetwork/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedNetworkMessages/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedObject/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedRandom/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedSynchronization/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedThread/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedUtility/include/public
    ${SWG_ENGINE_SOURCE_DIR}/server/library/serverDatabase/include/public
    ${SWG_ENGINE_SOURCE_DIR}/server/library/serverGame/include/public
    ${SWG_ENGINE_SOURCE_DIR}/server/library/serverMetrics/include/public
    ${SWG_ENGINE_SOURCE_DIR}/server/library/serverNetworkMessages/include/public
    ${SWG_ENGINE_SOURCE_DIR}/server/library/serverUtility/include/public
    ${SWG_GAME_SOURCE_DIR}/shared/library/swgSharedUtility/include/public
    ${SWG_EXTERNALS_SOURCE_DIR}/ours/library/archive/include
    ${SWG_GAME_SOURCE_DIR}/shared/library/swgSharedNetworkMessages/include/public
    ${SWG_GAME_SOURCE_DIR}/server/library/swgServerNetworkMessages/include/public
    ${SWG_EXTERNALS_SOURCE_DIR}/ours/library/fileInterface/include/public
    ${SWG_EXTERNALS_SOURCE_DIR}/ours/library/localization/include
    ${SWG_EXTERNALS_SOURCE_DIR}/ours/library/localizationArchive/include/public
    ${SWG_EXTERNALS_SOURCE_DIR}/ours/library/singleton/include
    ${SWG_EXTERNALS_SOURCE_DIR}/ours/library/unicode/include
    ${SWG_EXTERNALS_SOURCE_DIR}/ours/library/unicodeArchive/include/public
    ${SWG_EXTERNALS_SOURCE_DIR}/3rd/library/webAPI
)

set(MAKE_SCHEMA_H ${SWG_ENGINE_SOURCE_DIR}/server/library/codegen/make_schema_h.pl)
file(TO_NATIVE_PATH ${MAKE_SCHEMA_H} MAKE_SCHEMA_H_NATIVE)

set(MAKE_PACKAGES ${SWG_ENGINE_SOURCE_DIR}/server/library/codegen/make_packages.pl)
file(TO_NATIVE_PATH ${MAKE_PACKAGES} MAKE_PACKAGES_NATIVE)

set(MAKE_QUERIES ${SWG_ENGINE_SOURCE_DIR}/server/library/codegen/make_queries.pl)
file(TO_NATIVE_PATH ${MAKE_QUERIES} MAKE_QUERIES_NATIVE)

set(PACKAGE_DATA ${SWG_ENGINE_SOURCE_DIR}/server/library/codegen/package_data.txt)
file(TO_NATIVE_PATH ${PACKAGE_DATA} PACKAGE_DATA_NATIVE)



set(SCHEMA_H_TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/shared/generated/Schema_h.template)
file(TO_NATIVE_PATH ${SCHEMA_H_TEMPLATE} SCHEMA_H_TEMPLATE_NATIVE)

set(SCHEMA_H_OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/shared/generated/Schema.h)
file(TO_NATIVE_PATH ${SCHEMA_H_OUTPUT} SCHEMA_H_OUTPUT_NATIVE)


set(ENCODER_CPP_TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/shared/generated/Encoder_cpp.template)
file(TO_NATIVE_PATH ${ENCODER_CPP_TEMPLATE} ENCODER_CPP_TEMPLATE_NATIVE)

set(ENCODER_CPP_OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/shared/generated/Encoder.cpp)
file(TO_NATIVE_PATH ${ENCODER_CPP_OUTPUT} ENCODER_CPP_OUTPUT_NATIVE)


set(DECODER_CPP_TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/shared/generated/Decoder_cpp.template)
file(TO_NATIVE_PATH ${DECODER_CPP_TEMPLATE} DECODER_CPP_TEMPLATE_NATIVE)

set(DECODER_CPP_OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/shared/generated/Decoder.cpp)
file(TO_NATIVE_PATH ${DECODER_CPP_OUTPUT} DECODER_CPP_OUTPUT_NATIVE)


set(LOAD_OBJECT_CPP_TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/shared/generated/LoadObject_cpp.template)
file(TO_NATIVE_PATH ${LOAD_OBJECT_CPP_TEMPLATE} LOAD_OBJECT_CPP_TEMPLATE_NATIVE)

set(LOAD_OBJECT_CPP_OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/shared/generated/LoadObject.cpp)
file(TO_NATIVE_PATH ${LOAD_OBJECT_CPP_OUTPUT} LOAD_OBJECT_CPP_OUTPUT_NATIVE)


set(OBJECT_QUERIES_CPP_TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/shared/generated/ObjectQueries_cpp.template)
file(TO_NATIVE_PATH ${OBJECT_QUERIES_CPP_TEMPLATE} OBJECT_QUERIES_CPP_TEMPLATE_NATIVE)

set(OBJECT_QUERIES_CPP_OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/shared/generated/ObjectQueries.cpp)
file(TO_NATIVE_PATH ${OBJECT_QUERIES_CPP_OUTPUT} OBJECT_QUERIES_CPP_OUTPUT_NATIVE)


set(OBJECT_QUERIES_H_TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/shared/generated/ObjectQueries_h.template)
file(TO_NATIVE_PATH ${OBJECT_QUERIES_H_TEMPLATE} OBJECT_QUERIES_H_TEMPLATE_NATIVE)

set(OBJECT_QUERIES_H_OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/shared/generated/ObjectQueries.h)
file(TO_NATIVE_PATH ${OBJECT_QUERIES_H_OUTPUT} OBJECT_QUERIES_H_OUTPUT_NATIVE)

if(WIN32)
    set(WINDOWS_PLATFORM --windows)
endif()

add_custom_command(
    OUTPUT ${SCHEMA_H_OUTPUT_NATIVE}
    COMMAND ${CMAKE_COMMAND} -E copy ${SCHEMA_H_TEMPLATE_NATIVE} ${SCHEMA_H_OUTPUT_NATIVE}
    COMMAND ${PERL_EXECUTABLE} "${MAKE_SCHEMA_H}" --ddldirectory "${SWG_GAME_SOURCE_DIR}/server/database/schema" --output "${SCHEMA_H_OUTPUT_NATIVE}" ${WINDOWS_PLATFORM}
    DEPENDS ${SCHEMA_H_TEMPLATE_NATIVE} ${MAKE_SCHEMA_H}
)

add_custom_command(
    OUTPUT ${ENCODER_CPP_OUTPUT_NATIVE}
    COMMAND ${CMAKE_COMMAND} -E copy ${ENCODER_CPP_TEMPLATE_NATIVE} ${ENCODER_CPP_OUTPUT_NATIVE}
    COMMAND ${PERL_EXECUTABLE} "${MAKE_PACKAGES_NATIVE}" --datafile "${PACKAGE_DATA_NATIVE}" --encoder "${ENCODER_CPP_OUTPUT_NATIVE}" ${WINDOWS_PLATFORM}
    DEPENDS ${ENCODER_CPP_TEMPLATE_NATIVE} ${MAKE_PACKAGES_NATIVE} ${PACKAGE_DATA_NATIVE}
)

add_custom_command(
    OUTPUT ${DECODER_CPP_OUTPUT_NATIVE}
    COMMAND ${CMAKE_COMMAND} -E copy ${DECODER_CPP_TEMPLATE_NATIVE} ${DECODER_CPP_OUTPUT_NATIVE}
    COMMAND ${PERL_EXECUTABLE} "${MAKE_PACKAGES_NATIVE}" --datafile "${PACKAGE_DATA_NATIVE}" --decoder "${DECODER_CPP_OUTPUT_NATIVE}" ${WINDOWS_PLATFORM}
    DEPENDS ${DECODER_CPP_TEMPLATE_NATIVE} ${MAKE_PACKAGES_NATIVE} ${PACKAGE_DATA_NATIVE}
)

add_custom_command(
    OUTPUT ${LOAD_OBJECT_CPP_OUTPUT_NATIVE}
    COMMAND ${CMAKE_COMMAND} -E copy ${LOAD_OBJECT_CPP_TEMPLATE_NATIVE} ${LOAD_OBJECT_CPP_OUTPUT_NATIVE}
    COMMAND ${PERL_EXECUTABLE} "${MAKE_PACKAGES_NATIVE}" --datafile "${PACKAGE_DATA_NATIVE}" --loadobject "${LOAD_OBJECT_CPP_OUTPUT_NATIVE}" ${WINDOWS_PLATFORM}
    DEPENDS ${LOAD_OBJECT_CPP_TEMPLATE_NATIVE} ${MAKE_PACKAGES_NATIVE} ${PACKAGE_DATA_NATIVE}
)

add_custom_command(
    OUTPUT ${OBJECT_QUERIES_CPP_OUTPUT_NATIVE}
    COMMAND ${CMAKE_COMMAND} -E copy ${OBJECT_QUERIES_CPP_TEMPLATE_NATIVE} ${OBJECT_QUERIES_CPP_OUTPUT_NATIVE}
    COMMAND ${PERL_EXECUTABLE} "${MAKE_QUERIES}" --plsql "${SWG_GAME_SOURCE_DIR}/server/database/packages/persister.plsqlh" --source "${OBJECT_QUERIES_CPP_OUTPUT_NATIVE}" ${WINDOWS_PLATFORM}
    DEPENDS ${OBJECT_QUERIES_CPP_TEMPLATE_NATIVE} ${MAKE_QUERIES}
)

add_custom_command(
    OUTPUT ${OBJECT_QUERIES_H_OUTPUT_NATIVE}
    COMMAND ${CMAKE_COMMAND} -E copy ${OBJECT_QUERIES_H_TEMPLATE_NATIVE} ${OBJECT_QUERIES_H_OUTPUT_NATIVE}
    COMMAND ${PERL_EXECUTABLE} "${MAKE_QUERIES}" --plsql "${SWG_GAME_SOURCE_DIR}/server/database/packages/persister.plsqlh" --header "${OBJECT_QUERIES_H_OUTPUT_NATIVE}" ${WINDOWS_PLATFORM}
    DEPENDS ${OBJECT_QUERIES_H_TEMPLATE_NATIVE} ${MAKE_QUERIES}
)

add_executable(SwgDatabaseServer
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
    ${SCHEMA_H_OUTPUT}
    ${ENCODER_CPP_OUTPUT}
    ${DECODER_CPP_OUTPUT}
    ${LOAD_OBJECT_CPP_OUTPUT}
    ${OBJECT_QUERIES_CPP_OUTPUT}
    ${OBJECT_QUERIES_H_OUTPUT}
)

target_link_libraries(SwgDatabaseServer
    sharedCollision
    sharedCommandParser
    sharedCompression
    sharedDatabaseInterface
    sharedDatabaseInterface_oci
    sharedDebug
    sharedFile
    sharedFoundation
    sharedGame
    sharedLog
    sharedMath
    
    sharedMessageDispatch
    sharedNetwork
    sharedNetworkMessages
    sharedObject
    sharedRandom
    sharedSynchronization
    sharedTerrain
    sharedThread
    sharedUtility
    serverDatabase
    serverGame
    serverMetrics
    serverNetworkMessages
    serverUtility
    swgSharedUtility
    swgSharedNetworkMessages
    swgServerNetworkMessages
    archive
    fileInterface
    localization
    localizationArchive
    unicode
    unicodeArchive
    webAPI
    ${CURL_LIBRARIES}
    ${CMAKE_DL_LIBS}
)
