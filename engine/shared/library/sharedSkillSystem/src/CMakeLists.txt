
set(SHARED_SOURCES
	shared/ExpertiseManager.cpp
	shared/ExpertiseManager.h
	shared/LevelManager.cpp
	shared/LevelManager.h
	shared/SkillManager.cpp
	shared/SkillManager.h
	shared/SkillObjectArchive.cpp
	shared/SkillObjectArchive.h
	shared/SkillObject.cpp
	shared/SkillObject.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/FirstSharedSkillSystem.cpp
	)

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/win32)
else()
	set(PLATFORM_SOURCES "")
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/shared
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedDebug/include/public	
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFile/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundation/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundationTypes/include/public
	
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedUtility/include/public
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/archive/include
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/unicode/include
)

add_library(sharedSkillSystem STATIC
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)
