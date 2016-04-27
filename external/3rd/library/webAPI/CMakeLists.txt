set(SHARED_SOURCES
	webAPI.cpp
	webAPI.h
	json.hpp
)

include_directories(
	${CURL_INCLUDE_DIRS}
)

#target_link_libraries(
#	${CURL_LIBRARIES}
#)
