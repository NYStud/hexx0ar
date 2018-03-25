# - Try to find EPOXY
# Once done, this will define
#
#  EPOXY_FOUND - system has epoxy
#  EPOXY_INCLUDE_DIRS - the epoxy include directories
#  EPOXY_LIBRARIES - link these to use epoxy
#
# See documentation on how to write CMake scripts at
# http://www.cmake.org/Wiki/CMake:How_To_Find_Libraries

include(LibFindMacros)

libfind_pkg_check_modules(EPOXY_PKGCONF epoxy)

find_path(EPOXY_INCLUDE_DIR
        NAMES epoxy/gl.h
        HINTS ${EPOXY_PKGCONF_INCLUDE_DIRS}
        PATH_SUFFIXES EPOXY
        )

find_library(EPOXY_LIBRARY
        NAMES epoxy
        HINTS ${EPOXY_PKGCONF_LIBRARY_DIRS}
        )

set(EPOXY_PROCESS_INCLUDES EPOXY_INCLUDE_DIR)
set(EPOXY_PROCESS_LIBS EPOXY_LIBRARY)
set(EPOXY_VERSION ${EPOXY_PKGCONF_VERSION})
libfind_process(EPOXY)
