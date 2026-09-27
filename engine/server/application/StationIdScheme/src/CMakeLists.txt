include_directories(
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedDebug/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundation/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundationTypes/include/public
)

add_executable(stationIdScheme
	linux/main.cpp
)

target_link_libraries(stationIdScheme
    sharedFoundation
)
