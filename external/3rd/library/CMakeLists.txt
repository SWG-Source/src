add_subdirectory(webAPI)
add_subdirectory(platform)
add_subdirectory(soePlatform)
add_subdirectory(udplibrary)

if(${BUILD_MULODI_LIB})
    add_subdirectory(mulodi)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/webAPI)
