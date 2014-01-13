

add_subdirectory(shared)

if(WIN32)
	add_subdirectory(win32)
else()
	add_subdirectory(linux)
endif()