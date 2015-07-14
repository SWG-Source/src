
find_path(ICONV_ROOT
    NAMES include/iconv.h
)

find_path(ICONV_INCLUDE_DIR iconv.h
    HINTS
        $ENV{ICONV_ROOT}
    PATH_SUFFIXES include
    PATHS
        ${ICONV_ROOT}
        ${ICONV_INCLUDEDIR}
)

find_library(ICONV_LIBRARY
    NAMES iconv
    PATH_SUFFIXES lib
    HINTS
        $ENV{ICONV_ROOT}
        ${ICONV_ROOT}
        ${ICONV_LIBRARYDIR}
)

# handle the QUIETLY and REQUIRED arguments and set OPENAL_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ICONV DEFAULT_MSG ICONV_LIBRARY ICONV_INCLUDE_DIR)

mark_as_advanced(ICONV_ROOT ICONV_INCLUDE_DIR ICONV_LIBRARY)
