if (DEFINED _MIFF OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	add_subdirectory(application)
endif()
