

find_path(LIBXML2_INCLUDE_DIR libxml/xpath.h
    HINTS
        $ENV{LIBXML2_ROOT}
    PATH_SUFFIXES include include/libxml2 libxml2
    PATHS
        ${LIBXML2_ROOT}
        ${LIBXML2_INCLUDEDIR}
)

find_library(LIBXML2_LIBRARY
    NAMES xml2 libxml2
    PATH_SUFFIXES lib
    HINTS
        $ENV{LIBXML2_ROOT}
        ${LIBXML2_ROOT}
        ${LIBXML2_LIBRARYDIR}
)

# handle the QUIETLY and REQUIRED arguments and set OPENAL_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibXml2 DEFAULT_MSG LIBXML2_LIBRARY LIBXML2_INCLUDE_DIR)

mark_as_advanced(LIBXML2_ROOT LIBXML2_INCLUDE_DIR LIBXML2_LIBRARY)
