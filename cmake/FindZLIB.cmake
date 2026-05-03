# Prefer zlib built in-tree via FetchContent (see Dependencies.cmake) so MinGW/CI
# environments without a discoverable system ZLIB still satisfy libxlsxwriter.
if(TARGET zlibstatic)
    get_target_property(_currency_zlib_includes zlibstatic INTERFACE_INCLUDE_DIRECTORIES)
    if(_currency_zlib_includes STREQUAL "INTERFACE_INCLUDE_DIRECTORIES-NOTFOUND")
        set(ZLIB_INCLUDE_DIRS "")
    else()
        set(ZLIB_INCLUDE_DIRS "${_currency_zlib_includes}")
    endif()

    if(NOT TARGET ZLIB::ZLIB)
        add_library(ZLIB::ZLIB ALIAS zlibstatic)
    endif()
    set(ZLIB_LIBRARIES ZLIB::ZLIB)
    set(ZLIB_FOUND TRUE)

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(ZLIB REQUIRED_VARS ZLIB_LIBRARIES ZLIB_INCLUDE_DIRS)
else()
    include("${CMAKE_ROOT}/Modules/FindZLIB.cmake")
endif()
