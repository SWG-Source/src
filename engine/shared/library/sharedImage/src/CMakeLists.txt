
set(SHARED_SOURCES
	shared/FirstSharedImage.h
	shared/ConfigSharedImage.cpp
	shared/ConfigSharedImage.h
	shared/Image.cpp
	shared/ImageFormat.cpp
	shared/ImageFormat.h
	shared/ImageFormatList.cpp
	shared/ImageFormatList.h
	shared/Image.h
	shared/ImageManipulation.cpp
	shared/ImageManipulation.h
	shared/SetupSharedImage.cpp
	shared/SetupSharedImage.h
	shared/TargaFormat.cpp
	shared/TargaFormat.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/FirstSharedImage.cpp
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
	
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/fileInterface/include/public
)

add_library(sharedImage STATIC
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)
