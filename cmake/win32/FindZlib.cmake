
find_path(ZLIB_INCLUDE_DIR
	PATHS ${SWG_EXTERNALS_FIND}/Zlib/include
 	NAMES zlib.h )
	
	
  find_library(ZLIB_LIBRARY  
	NAMES zlib 
	PATHS ${SWG_EXTERNALS_FIND}/Zlib/lib/win32) 

# handle the QUIETLY and REQUIRED arguments and set OPENAL_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ZLIB DEFAULT_MSG ZLIB_INCLUDE_DIR ZLIB_LIBRARY)

mark_as_advanced(ZLIB_INCLUDE_DIR ZLIB_LIBRARY)

