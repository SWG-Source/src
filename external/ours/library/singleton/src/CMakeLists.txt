
set(SHARED_SOURCES
	shared/Singleton.h
	shared/Singleton2.h
)

add_library(singleton STATIC ${EXCLUDE_PROJECT}
	${SHARED_SOURCES}
)

set_target_properties(singleton PROPERTIES LINKER_LANGUAGE CXX)
