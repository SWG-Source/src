find_path(PCRE_INCLUDE_DIR pcre.h
    HINTS
        $ENV{PCRE_ROOT}
    PATH_SUFFIXES include
    PATHS
        ${PCRE_ROOT}
        ${PCRE_INCLUDEDIR}
)

find_library(PCRE_LIBRARY
    NAMES pcre libpcre
    PATH_SUFFIXES lib
    HINTS
        $ENV{PCRE_ROOT}
        ${PCRE_ROOT}
        ${PCRE_LIBRARYDIR}
)

# handle the QUIETLY and REQUIRED arguments and set OPENAL_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PCRE DEFAULT_MSG PCRE_LIBRARY PCRE_INCLUDE_DIR)

mark_as_advanced(PCRE_ROOT PCRE_INCLUDE_DIR PCRE_LIBRARY)
