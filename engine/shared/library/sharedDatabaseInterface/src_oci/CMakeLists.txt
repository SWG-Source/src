
set(SHARED_SOURCES
	DbBindableVarray.cpp
	DbBindableVarray.h
	OciQueryImplementation.cpp
	OciQueryImplementation.h
	OciServer.cpp
	OciServer.h
	OciSession.cpp
	OciSession.h
)

include_directories(
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedDebug/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundation/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundationTypes/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedLog/include/public
	
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedSynchronization/include/public
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/unicode/include
	${ORACLE_INCLUDE_DIR}
)

add_library(sharedDatabaseInterface_oci STATIC
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)

target_link_libraries(sharedDatabaseInterface_oci
	sharedDatabaseInterface
    ${ORACLE_LIBRARY}
)
