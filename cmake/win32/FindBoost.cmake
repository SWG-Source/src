
find_path(Boost_INCLUDE_DIR
	PATHS ${SWG_EXTERNALS_FIND}
	PATH_SUFFIXES boost
	NAMES boost/version.hpp )
	
	
find_path(BOOST_ROOT
	PATHS ${SWG_EXTERNALS_FIND}
	PATH_SUFFIXES boost
	NAMES boost/version.hpp )

# handle the QUIETLY and REQUIRED arguments and set OPENAL_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ZLIB DEFAULT_MSG ZLIB_INCLUDE_DIR ZLIB_LIBRARY)

mark_as_advanced(ZLIB_INCLUDE_DIR ZLIB_LIBRARY)