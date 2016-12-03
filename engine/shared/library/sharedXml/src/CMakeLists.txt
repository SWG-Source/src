
set(SHARED_SOURCES
	shared/core/FirstSharedXml.h
	shared/core/SetupSharedXml.cpp
	shared/core/SetupSharedXml.h

	shared/tree/XmlTreeDocument.cpp
	shared/tree/XmlTreeDocument.h
	shared/tree/XmlTreeDocumentList.cpp
	shared/tree/XmlTreeDocumentList.h
	shared/tree/XmlTreeNode.cpp
	shared/tree/XmlTreeNode.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/core/FirstSharedXml.cpp
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
	${ICONV_INCLUDE_DIR}
	${LIBXML2_INCLUDE_DIR}
)

add_library(sharedXml STATIC
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)

target_link_libraries(sharedXml
	${LIBXML2_LIBRARY}
)
