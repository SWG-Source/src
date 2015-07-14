
find_path(ORACLE_ROOT
	PATHS ${SWG_EXTERNALS_FIND}
	PATH_SUFFIXES OCI
 	NAMES include/oci.h )
	
find_path(ORACLE_INCLUDE_DIR
	PATHS ${SWG_EXTERNALS_FIND}/OCI/include
 	NAMES oci.h )
	
find_path(BOOST_ROOT
	PATHS ${SWG_EXTERNALS_FIND}
	PATH_SUFFIXES boost
	NAMES boost/version.hpp )
	
 set(ORACLE_OCI_NAMES clntsh libclntsh oci)
 set(ORACLE_NNZ_NAMES nnz10 libnnz10 nnz11 libnnz11 nnz12 libnnz12 ociw32)
 set(ORACLE_OCCI_NAMES libocci occi oraocci10 oraocci11 oraocci12)

  set(ORACLE_LIB_DIR 
    ${ORACLE_ROOT}/lib/msvc)

	
  find_library(ORACLE_LIBRARY  NAMES ${ORACLE_OCI_NAMES} PATHS ${ORACLE_LIB_DIR})
  find_library(ORACLE_OCCI_LIBRARY NAMES ${ORACLE_OCCI_NAMES} PATHS ${ORACLE_LIB_DIR})
  find_library(ORACLE_NNZ_LIBRARY NAMES ${ORACLE_NNZ_NAMES} PATHS ${ORACLE_LIB_DIR})
  

# handle the QUIETLY and REQUIRED arguments and set OPENAL_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ORACLE ORACLE_ROOT DEFAULT_MSG ORACLE_LIBRARY ORACLE_INCLUDE_DIR BOOST_ROOT)

mark_as_advanced(ORACLE_INCLUDE_DIR ORACLE_LIBRARY BOOST_ROOT ORACLE_ROOT)

