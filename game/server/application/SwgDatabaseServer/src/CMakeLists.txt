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
    ${CMAKE_CURRENT_BINARY_DIR}/generated
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
# Schema.h is generated from every table definition, so a changed .tab file
# (a new column, or a BIND_AS that changes a column's C++ type) regenerates it.
file(TO_NATIVE_PATH ${MAKE_SCHEMA_H} MAKE_SCHEMA_H_NATIVE)

set(MAKE_PACKAGES ${SWG_ENGINE_SOURCE_DIR}/server/library/codegen/make_packages.pl)
file(TO_NATIVE_PATH ${MAKE_PACKAGES} MAKE_PACKAGES_NATIVE)

set(MAKE_QUERIES ${SWG_ENGINE_SOURCE_DIR}/server/library/codegen/make_queries.pl)
file(TO_NATIVE_PATH ${MAKE_QUERIES} MAKE_QUERIES_NATIVE)

set(PACKAGE_DATA ${SWG_ENGINE_SOURCE_DIR}/server/library/codegen/package_data.txt)
file(TO_NATIVE_PATH ${PACKAGE_DATA} PACKAGE_DATA_NATIVE)

set(PERSISTER_PLSQLH ${SWG_GAME_SOURCE_DIR}/server/database/packages/persister.plsqlh)
file(TO_NATIVE_PATH ${PERSISTER_PLSQLH} PERSISTER_PLSQLH_NATIVE)

set(SCHEMA_DIRECTORY ${SWG_GAME_SOURCE_DIR}/server/database/schema)
# CONFIGURE_DEPENDS re-globs at build time so that added tables are seen.
if(CMAKE_VERSION VERSION_LESS 3.12)
    message(FATAL_ERROR "SwgDatabaseServer schema generation requires CMake 3.12 or newer")
endif()
file(GLOB SCHEMA_TABLES CONFIGURE_DEPENDS ${SCHEMA_DIRECTORY}/*.tab)

# Removing a table must also regenerate Schema.h, so the table list is an input
# too; configure_file rewrites it only when the list changes.
set(SCHEMA_TABLE_LIST ${CMAKE_CURRENT_BINARY_DIR}/schema_tables.txt)
string(REPLACE ";" "\n" SCHEMA_TABLE_LINES "${SCHEMA_TABLES}")
file(WRITE ${SCHEMA_TABLE_LIST}.in "${SCHEMA_TABLE_LINES}\n")
configure_file(${SCHEMA_TABLE_LIST}.in ${SCHEMA_TABLE_LIST} COPYONLY)

# Generated sources belong to this build directory, not the source tree, so
# that build directories never share or race on them.  Each generator edits a
# private copy of its template which is renamed into place only once complete.
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated/SwgDatabaseServer)
file(MAKE_DIRECTORY ${GENERATED_DIR})


set(SCHEMA_H_TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/shared/generated/Schema_h.template)
file(TO_NATIVE_PATH ${SCHEMA_H_TEMPLATE} SCHEMA_H_TEMPLATE_NATIVE)

set(SCHEMA_H_OUTPUT ${GENERATED_DIR}/Schema.h)
file(TO_NATIVE_PATH ${SCHEMA_H_OUTPUT} SCHEMA_H_OUTPUT_NATIVE)


set(ENCODER_CPP_TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/shared/generated/Encoder_cpp.template)
file(TO_NATIVE_PATH ${ENCODER_CPP_TEMPLATE} ENCODER_CPP_TEMPLATE_NATIVE)

set(ENCODER_CPP_OUTPUT ${GENERATED_DIR}/Encoder.cpp)
file(TO_NATIVE_PATH ${ENCODER_CPP_OUTPUT} ENCODER_CPP_OUTPUT_NATIVE)


set(DECODER_CPP_TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/shared/generated/Decoder_cpp.template)
file(TO_NATIVE_PATH ${DECODER_CPP_TEMPLATE} DECODER_CPP_TEMPLATE_NATIVE)

set(DECODER_CPP_OUTPUT ${GENERATED_DIR}/Decoder.cpp)
file(TO_NATIVE_PATH ${DECODER_CPP_OUTPUT} DECODER_CPP_OUTPUT_NATIVE)


set(LOAD_OBJECT_CPP_TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/shared/generated/LoadObject_cpp.template)
file(TO_NATIVE_PATH ${LOAD_OBJECT_CPP_TEMPLATE} LOAD_OBJECT_CPP_TEMPLATE_NATIVE)

set(LOAD_OBJECT_CPP_OUTPUT ${GENERATED_DIR}/LoadObject.cpp)
file(TO_NATIVE_PATH ${LOAD_OBJECT_CPP_OUTPUT} LOAD_OBJECT_CPP_OUTPUT_NATIVE)


set(OBJECT_QUERIES_CPP_TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/shared/generated/ObjectQueries_cpp.template)
file(TO_NATIVE_PATH ${OBJECT_QUERIES_CPP_TEMPLATE} OBJECT_QUERIES_CPP_TEMPLATE_NATIVE)

set(OBJECT_QUERIES_CPP_OUTPUT ${GENERATED_DIR}/ObjectQueries.cpp)
file(TO_NATIVE_PATH ${OBJECT_QUERIES_CPP_OUTPUT} OBJECT_QUERIES_CPP_OUTPUT_NATIVE)


set(OBJECT_QUERIES_H_TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/shared/generated/ObjectQueries_h.template)
file(TO_NATIVE_PATH ${OBJECT_QUERIES_H_TEMPLATE} OBJECT_QUERIES_H_TEMPLATE_NATIVE)

set(OBJECT_QUERIES_H_OUTPUT ${GENERATED_DIR}/ObjectQueries.h)
file(TO_NATIVE_PATH ${OBJECT_QUERIES_H_OUTPUT} OBJECT_QUERIES_H_OUTPUT_NATIVE)

if(WIN32)
    set(WINDOWS_PLATFORM --windows)
endif()

add_custom_command(
    OUTPUT ${SCHEMA_H_OUTPUT_NATIVE}
    COMMAND ${CMAKE_COMMAND} -E copy ${SCHEMA_H_TEMPLATE_NATIVE} ${SCHEMA_H_OUTPUT_NATIVE}.tmp
    COMMAND ${PERL_EXECUTABLE} "${MAKE_SCHEMA_H_NATIVE}" --ddldirectory "${SCHEMA_DIRECTORY}" --output "${SCHEMA_H_OUTPUT_NATIVE}.tmp" ${WINDOWS_PLATFORM}
    COMMAND ${CMAKE_COMMAND} -E rename ${SCHEMA_H_OUTPUT_NATIVE}.tmp ${SCHEMA_H_OUTPUT_NATIVE}
    DEPENDS ${SCHEMA_H_TEMPLATE_NATIVE} ${MAKE_SCHEMA_H_NATIVE} ${SCHEMA_TABLE_LIST} ${SCHEMA_TABLES}
)

add_custom_command(
    OUTPUT ${ENCODER_CPP_OUTPUT_NATIVE}
    COMMAND ${CMAKE_COMMAND} -E copy ${ENCODER_CPP_TEMPLATE_NATIVE} ${ENCODER_CPP_OUTPUT_NATIVE}.tmp
    COMMAND ${PERL_EXECUTABLE} "${MAKE_PACKAGES_NATIVE}" --datafile "${PACKAGE_DATA_NATIVE}" --encoder "${ENCODER_CPP_OUTPUT_NATIVE}.tmp" ${WINDOWS_PLATFORM}
    COMMAND ${CMAKE_COMMAND} -E rename ${ENCODER_CPP_OUTPUT_NATIVE}.tmp ${ENCODER_CPP_OUTPUT_NATIVE}
    DEPENDS ${ENCODER_CPP_TEMPLATE_NATIVE} ${MAKE_PACKAGES_NATIVE} ${PACKAGE_DATA_NATIVE}
)

add_custom_command(
    OUTPUT ${DECODER_CPP_OUTPUT_NATIVE}
    COMMAND ${CMAKE_COMMAND} -E copy ${DECODER_CPP_TEMPLATE_NATIVE} ${DECODER_CPP_OUTPUT_NATIVE}.tmp
    COMMAND ${PERL_EXECUTABLE} "${MAKE_PACKAGES_NATIVE}" --datafile "${PACKAGE_DATA_NATIVE}" --decoder "${DECODER_CPP_OUTPUT_NATIVE}.tmp" ${WINDOWS_PLATFORM}
    COMMAND ${CMAKE_COMMAND} -E rename ${DECODER_CPP_OUTPUT_NATIVE}.tmp ${DECODER_CPP_OUTPUT_NATIVE}
    DEPENDS ${DECODER_CPP_TEMPLATE_NATIVE} ${MAKE_PACKAGES_NATIVE} ${PACKAGE_DATA_NATIVE}
)

add_custom_command(
    OUTPUT ${LOAD_OBJECT_CPP_OUTPUT_NATIVE}
    COMMAND ${CMAKE_COMMAND} -E copy ${LOAD_OBJECT_CPP_TEMPLATE_NATIVE} ${LOAD_OBJECT_CPP_OUTPUT_NATIVE}.tmp
    COMMAND ${PERL_EXECUTABLE} "${MAKE_PACKAGES_NATIVE}" --datafile "${PACKAGE_DATA_NATIVE}" --loadobject "${LOAD_OBJECT_CPP_OUTPUT_NATIVE}.tmp" ${WINDOWS_PLATFORM}
    COMMAND ${CMAKE_COMMAND} -E rename ${LOAD_OBJECT_CPP_OUTPUT_NATIVE}.tmp ${LOAD_OBJECT_CPP_OUTPUT_NATIVE}
    DEPENDS ${LOAD_OBJECT_CPP_TEMPLATE_NATIVE} ${MAKE_PACKAGES_NATIVE} ${PACKAGE_DATA_NATIVE}
)

add_custom_command(
    OUTPUT ${OBJECT_QUERIES_CPP_OUTPUT_NATIVE}
    COMMAND ${CMAKE_COMMAND} -E copy ${OBJECT_QUERIES_CPP_TEMPLATE_NATIVE} ${OBJECT_QUERIES_CPP_OUTPUT_NATIVE}.tmp
    COMMAND ${PERL_EXECUTABLE} "${MAKE_QUERIES_NATIVE}" --plsql "${PERSISTER_PLSQLH_NATIVE}" --source "${OBJECT_QUERIES_CPP_OUTPUT_NATIVE}.tmp" ${WINDOWS_PLATFORM}
    COMMAND ${CMAKE_COMMAND} -E rename ${OBJECT_QUERIES_CPP_OUTPUT_NATIVE}.tmp ${OBJECT_QUERIES_CPP_OUTPUT_NATIVE}
    DEPENDS ${OBJECT_QUERIES_CPP_TEMPLATE_NATIVE} ${MAKE_QUERIES_NATIVE} ${PERSISTER_PLSQLH_NATIVE}
)

add_custom_command(
    OUTPUT ${OBJECT_QUERIES_H_OUTPUT_NATIVE}
    COMMAND ${CMAKE_COMMAND} -E copy ${OBJECT_QUERIES_H_TEMPLATE_NATIVE} ${OBJECT_QUERIES_H_OUTPUT_NATIVE}.tmp
    COMMAND ${PERL_EXECUTABLE} "${MAKE_QUERIES_NATIVE}" --plsql "${PERSISTER_PLSQLH_NATIVE}" --header "${OBJECT_QUERIES_H_OUTPUT_NATIVE}.tmp" ${WINDOWS_PLATFORM}
    COMMAND ${CMAKE_COMMAND} -E rename ${OBJECT_QUERIES_H_OUTPUT_NATIVE}.tmp ${OBJECT_QUERIES_H_OUTPUT_NATIVE}
    DEPENDS ${OBJECT_QUERIES_H_TEMPLATE_NATIVE} ${MAKE_QUERIES_NATIVE} ${PERSISTER_PLSQLH_NATIVE}
)

# Both the server and its snapshot test compile these generated sources.
# Give their custom commands one owner so parallel target builds cannot run
# two generators against the same temporary output files.
add_custom_target(swg_database_codegen DEPENDS
    ${SCHEMA_H_OUTPUT}
    ${ENCODER_CPP_OUTPUT}
    ${DECODER_CPP_OUTPUT}
    ${LOAD_OBJECT_CPP_OUTPUT}
    ${OBJECT_QUERIES_CPP_OUTPUT}
    ${OBJECT_QUERIES_H_OUTPUT}
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

add_dependencies(SwgDatabaseServer swg_database_codegen)

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

# ----------------------------------------------------------------------
# Tests of the 32-bit database bindings. Not built by default:
#   make int32_db_binding_test int32_snapshot_send_test
# They connect to a scratch schema named by the environment; see
# game/server/database/tools/int32_audit/test/README.md.

set(INT32_TEST_DIR ${SWG_GAME_SOURCE_DIR}/server/database/tools/int32_audit/test)
get_target_property(SWG_DATABASE_SERVER_LIBRARIES SwgDatabaseServer LINK_LIBRARIES)

add_executable(int32_db_binding_test EXCLUDE_FROM_ALL
    ${INT32_TEST_DIR}/db_binding_test.cpp
    ${INT32_TEST_DIR}/TestConnection.h
)
target_link_libraries(int32_db_binding_test ${SWG_DATABASE_SERVER_LIBRARIES})

# The SwgDatabaseServer sources without its main(), and the test's main().
add_executable(int32_snapshot_send_test EXCLUDE_FROM_ALL
    ${SHARED_SOURCES}
    ${SCHEMA_H_OUTPUT}
    ${ENCODER_CPP_OUTPUT}
    ${DECODER_CPP_OUTPUT}
    ${LOAD_OBJECT_CPP_OUTPUT}
    ${OBJECT_QUERIES_CPP_OUTPUT}
    ${OBJECT_QUERIES_H_OUTPUT}
    ${INT32_TEST_DIR}/snapshot_send_test.cpp
    ${INT32_TEST_DIR}/TestConnection.h
)
add_dependencies(int32_snapshot_send_test swg_database_codegen)

# The server's main() pulls some library objects in first; without it the
# static libraries need to be searched as a group.
target_link_libraries(int32_snapshot_send_test -Wl,--start-group ${SWG_DATABASE_SERVER_LIBRARIES} -Wl,--end-group)
