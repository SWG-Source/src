
set(SHARED_SOURCES
	shared/FirstSharedMathArchive.h
	shared/QuaternionArchive.h
	shared/SphereArchive.h
	shared/TransformArchive.h
	shared/VectorArchive.h
)

if(WIN32)
	set(EXCLUDE_PROJECT "")
else()
	set(EXCLUDE_PROJECT "EXCLUDE_FROM_ALL")
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/shared
	#${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedDebug/include/public
	#${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundation/include/public
	#${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundationTypes/include/public
	#
)

add_library(sharedMathArchive STATIC ${EXCLUDE_PROJECT}
	${SHARED_SOURCES}
)

set_target_properties(sharedMathArchive PROPERTIES LINKER_LANGUAGE CXX)
