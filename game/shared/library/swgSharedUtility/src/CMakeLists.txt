
set(SHARED_SOURCES
	shared/Attributes.cpp
	shared/Attributes.def
	shared/Attributes.h
	shared/Behaviors.def
	shared/CombatEngineData.cpp
	shared/CombatEngineData.h
	shared/FirstSwgSharedUtility.h
	shared/JediConstants.h
	shared/JediConstants.cpp
	shared/Locomotions.cpp
	shared/Locomotions.def
	shared/Locomotions.h
	shared/MentalStates.def
	shared/Postures.cpp
	shared/Postures.def
	shared/Postures.h
	shared/SpeciesRestrictions.cpp
	shared/SpeciesRestrictions.h
	shared/States.cpp
	shared/States.def
	shared/States.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/FirstSwgSharedUtility.cpp
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
	
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedObject/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedUtility/include/public
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/localization/include
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/unicode/include
)

add_library(swgSharedUtility STATIC
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)
