
set(SHARED_SOURCES
	shared/FirstServerKeyShare.h
	shared/KeyServer.cpp
	shared/KeyServer.h
	shared/KeyShare.cpp
	shared/KeyShare.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/FirstServerKeyShare.cpp
	)
else()
	set(PLATFORM_SOURCES "")
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/shared
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedDebug/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundation/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundationTypes/include/public
	
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedRandom/include/public
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/archive/include
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/crypto/include
)

add_library(serverKeyShare STATIC
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)
