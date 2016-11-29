
set(SHARED_SOURCES
    shared/DataTableTool.cpp
    shared/DataTableTool.h
    shared/FirstDataTableTool.cpp
    shared/FirstDataTableTool.h
)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/shared
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedCompression/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedDebug/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFile/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundation/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundationTypes/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedIoWin/include/public
    
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedObject/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedRandom/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedRegex/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedTemplate/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedTemplateDefinition/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedThread/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedUtility/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedXml/include/public
    ${SWG_EXTERNALS_SOURCE_DIR}/ours/library/archive/include
    ${SWG_EXTERNALS_SOURCE_DIR}/ours/library/fileInterface/include/public
    ${SWG_EXTERNALS_SOURCE_DIR}/ours/library/localization/include
    ${SWG_EXTERNALS_SOURCE_DIR}/ours/library/unicode/include
)

add_executable(DataTableTool
	${SHARED_SOURCES}
)

target_link_libraries(DataTableTool
    sharedCompression
    sharedDebug
    sharedFile
    sharedFoundation
    sharedIoWin
    sharedLog
    
    sharedMessageDispatch
    sharedObject
    sharedRandom
    sharedRegex
    sharedTemplate
    sharedTemplateDefinition
    sharedThread
    sharedUtility
    sharedXml
    archive
    localization
    localizationArchive
    unicode
    unicodeArchive
    ${CMAKE_DL_LIBS}
)
