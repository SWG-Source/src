
if(WIN32)
	set(PLATFORM_SOURCES
        win32/InputFileHandler.cpp
        win32/InputFileHandler.h
        win32/OutputFileHandler.cpp
        win32/OutputFileHandler.h
        win32/miff.cpp
	)

    set(PLATFORM win32)
else()
	set(PLATFORM_SOURCES
        linux/InputFileHandler.cpp
        linux/InputFileHandler.h
        linux/OutputFileHandler.cpp
        linux/OutputFileHandler.h
        linux/miff.cpp
	)

    set(PLATFORM linux)
endif()

include_directories(
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedCompression/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedDebug/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFile/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundation/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundationTypes/include/public
    
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedThread/include/public
    ${SWG_EXTERNALS_SOURCE_DIR}/ours/library/fileInterface/include/public
)

set(PARSER_LEX ${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM}/parser.lex)
set(PARSER_YAC ${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM}/parser.yac)
set(LEX_YY_C ${CMAKE_CURRENT_BINARY_DIR}/lex_yy.c)
set(PARSER_C ${CMAKE_CURRENT_BINARY_DIR}/parser.c)
set(PARSER_H ${CMAKE_CURRENT_BINARY_DIR}/parser.h)

if(WIN32)
    set(WORKING_DIR ${CMAKE_CURRENT_SOURCE_DIR}/win32)
endif()

add_custom_command(
    OUTPUT ${LEX_YY_C}
    COMMAND ${FLEX_EXECUTABLE} -w -o${LEX_YY_C} ${PARSER_LEX}
    DEPENDS ${PARSER_LEX}
)

add_custom_command(
    OUTPUT ${PARSER_C} ${PARSER_H}
    COMMAND ${BISON_EXECUTABLE} -d -o ${PARSER_C} ${PARSER_YAC}
    WORKING_DIRECTORY ${WORKING_DIR}
    DEPENDS ${PARSER_YAC}
)

add_executable(Miff
	${PLATFORM_SOURCES}
    ${LEX_YY_C}
    ${PARSER_C}
    ${PARSER_H}
)

target_link_libraries(Miff
    sharedCompression
    sharedDebug
    sharedFile
    sharedFoundation
    sharedLog
    
    sharedMessageDispatch
    sharedThread
    fileInterface
    ${CMAKE_DL_LIBS}
)
