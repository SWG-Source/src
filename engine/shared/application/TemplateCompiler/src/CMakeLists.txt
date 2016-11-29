
set(SHARED_SOURCES
    shared/TemplateCompiler.cpp
)

if(WIN32)
	set(PLATFORM_SOURCES
        win32/FirstTemplateCompiler.cpp
        win32/FirstTemplateCompiler.cpp
	)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/win32)
else()
	set(PLATFORM_SOURCES
        linux/FirstTemplateCompiler.cpp
        linux/FirstTemplateCompiler.cpp
	)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/linux)
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/shared
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedCompression/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedDebug/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFile/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundation/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundationTypes/include/public
    
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedObject/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedRandom/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedRegex/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedTemplate/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedTemplateDefinition/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedThread/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedUtility/include/public
    ${SWG_EXTERNALS_SOURCE_DIR}/ours/library/archive/include
    ${SWG_EXTERNALS_SOURCE_DIR}/ours/library/localization/include
    ${SWG_EXTERNALS_SOURCE_DIR}/ours/library/unicode/include
    ${PCRE_INCLUDE_DIR}
)

add_executable(TemplateCompiler
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)

target_link_libraries(TemplateCompiler
    sharedCompression
    sharedDebug
    sharedFile
    sharedFoundation
    sharedLog
    
    sharedMessageDispatch
    sharedObject
    sharedRandom
    sharedRegex
    sharedTemplate
    sharedTemplateDefinition
    sharedThread
    sharedUtility
    archive
    localization
    localizationArchive
    unicode
    unicodeArchive
    ${CMAKE_DL_LIBS}
)
