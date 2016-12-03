
set(SHARED_SOURCES
	shared/core/CompilerDynamicVariableParamData.cpp
	shared/core/CompilerDynamicVariableParamData.h
	shared/core/CompilerIntegerParam.cpp
	shared/core/CompilerIntegerParam.h
	shared/core/File.cpp
	shared/core/File.h
	shared/core/Filename.cpp
	shared/core/Filename.h
	shared/core/FirstSharedTemplateDefinition.h
	shared/core/ObjectTemplate.cpp
	shared/core/ObjectTemplate.h
	shared/core/TemplateData.cpp
	shared/core/TemplateData.h
	shared/core/TemplateDataIterator.cpp
	shared/core/TemplateDataIterator.h
	shared/core/TemplateDefinitionFile.cpp
	shared/core/TemplateDefinitionFile.h
	shared/core/TemplateGlobals.cpp
	shared/core/TemplateGlobals.h
	shared/core/TpfFile.cpp
	shared/core/TpfFile.h
	shared/core/TpfTemplate.cpp
	shared/core/TpfTemplate.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/FirstSharedTemplateDefinition.cpp
	)
else()
	set(PLATFORM_SOURCES
		linux/FirstSharedTemplateDefinition.cpp
	)
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/shared/core
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedDebug/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFile/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundation/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundationTypes/include/public
	
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedRandom/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedRegex/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedUtility/include/public
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/archive/include
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/localization/include
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/unicode/include
	${PCRE_INCLUDE_DIR}
)

add_library(sharedTemplateDefinition STATIC
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)
