cmake_minimum_required(VERSION 3.10)
if(POLICY CMP0072)
  cmake_policy(SET CMP0072 NEW)
endif()

if(POLICY CMP0093)
  cmake_policy(SET CMP0093 NEW)
endif()

#set(CMAKE_VERBOSE_MAKEFILE TRUE)

include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckLibraryExists)

set(CMAKE_GNUtoMS ON)
project(ZeroXml C CXX)
set(PACKAGE_NAME "ZeroXml")
set(PACKAGE "zeroxml")

option(UTILS  "Build and install utility programs"            ON)
option(WERROR "Treat compile warnings as errors"              OFF)
option(RMALLOC "Enable memory debugging functions"            OFF)

if(RMALLOC)
  set(USE_RMALLOC 1)
endif(RMALLOC)

if(WIN32)
  set(LIBZEROXML ZeroXML)
  set(LIBZEROXML_DEBUG ZeroXML-rmalloc)
  add_definitions("-D_WIN32")
  add_definitions("-DXML_BUILD_LIBRARY=1")
  set(CMAKE_INSTALL_PREFIX "C:/Program\ Files/${PACKAGE_NAME}")
else()
  set(LIBZEROXML zeroxml)
  set(LIBZEROXML_DEBUG zeroxml-rmalloc)
endif()

set(LIBTYPE STATIC)

# detect system type
if(NOT DEFINED CPACK_SYSTEM_NAME)
  set(CPACK_SYSTEM_NAME ${CMAKE_SYSTEM_NAME})
endif(NOT DEFINED CPACK_SYSTEM_NAME)
if(UNIX AND NOT WIN32)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(CPACK_PACKAGE_ARCHITECTURE "x86_64")
  else(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(CPACK_PACKAGE_ARCHITECTURE "i386")
  endif(CMAKE_SIZEOF_VOID_P EQUAL 8)
endif(UNIX AND NOT WIN32)

# read 'version' file into a variable (stripping any newlines or spaces)
file(READ version versionFile)
string(STRIP ${versionFile} ZEROXML_VERSION)
set(VERSION ZEROXML_VERSION)

#  read 'description` file into a variable
file(STRINGS description descriptionFile)
string(REGEX REPLACE ";" " " descriptionFile "${descriptionFile}")

# split version string into components, note CMAKE_MATCH_0 is the entire regexp match
string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)" CPACK_PACKAGE_VERSION ${ZEROXML_VERSION})
set(XML_MAJOR_VERSION ${CMAKE_MATCH_1})
set(XML_MINOR_VERSION ${CMAKE_MATCH_2})
set(XML_MICRO_VERSION ${CMAKE_MATCH_3})
set(CPACK_PACKAGE_VERSION_MAJOR ${CMAKE_MATCH_1})
set(CPACK_PACKAGE_VERSION_MINOR ${CMAKE_MATCH_2})
set(CPACK_PACKAGE_VERSION_PATCH ${CMAKE_MATCH_3})

set(CPACK_PACKAGE_NAME "${PACKAGE}")
set(CPACK_PACKAGE_file_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}.el6.${CPACK_PACKAGE_ARCHITECTURE}")
set(CPACK_PACKAGE_VENDOR "Adalin B.V.")
set(CPACK_PACKAGE_CONTACT "tech@adalin.org")
set(CPACK_PACKAGE_DESCRIPTION "A lightweight, cross platform XML configuration file reading library")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${descriptionFile})
set(CPACK_PACKAGE_DESCRIPTION_file "${PROJECT_SOURCE_DIR}/description")
set(CPACK_RESOURCE_file_LICENSE "${PROJECT_SOURCE_DIR}/COPYING")
set(CPACK_RESOURCE_file_README "${PROJECT_SOURCE_DIR}/README")

if(WIN32)

else(WIN32)
  # DEBIAN
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
  else(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386")
  endif(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${PACK_PACKAGE_ARCHITECTURE})
  set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.1)")
endif(WIN32)

# RPM
set(CPACK_RPM_PACKAGE_ARCHITECTURE ${PACK_PACKAGE_ARCHITECTURE})
set(CPACK_RPM_PACKAGE_GROUP "System/Libraries")


set(CPACK_SOURCE_GENERATOR ZIP)
set(CPACK_SOURCE_PACKAGE_file_NAME "${CMAKE_PROJECT_NAME}-${ZEROXML_VERSION}" CACHE INTERNAL "tarball basename")
set(CPACK_SOURCE_IGNORE_fileS
  "^${PROJECT_BINARY_DIR};${PROJECT_SOURCE_DIR}/.git;\\\\.gitignore;Makefile.am;~$;${CPACK_SOURCE_IGNORE_fileS}")

set(CPACK_GENERATOR "DEB;RPM")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Adalin B.V.")

include(CPack)

# We have some custom .cmake scripts not in the official distribution.
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")

include(CPack_Defaults)

# Add definitions, compiler switches, etc.
include_directories("${CMAKE_SOURCE_DIR}/src"
  "${CMAKE_BINARY_DIR}/include"
  include
  src 
)

if(NOT WIN32)
check_include_file(iconv.h HAVE_ICONV_H)
endif()
check_include_file(strings.h HAVE_STRINGS_H)
check_include_file(unistd.h HAVE_UNISTD_H)
check_include_file(locale.h HAVE_LOCALE_H)
check_include_file(langinfo.h HAVE_LANGINFO_H)

add_definitions(-DHAVE_CONFIG_H=1)
if(WERROR)
  add_definitions(-Werror)
endif()

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release")
endif()


if(MSVC)
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -D_DEBUG")
    set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -DNDEBUG")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DNDEBUG")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
    add_definitions(/NODEFAULTLIB:libcmt.lib)
    add_definitions(/NODEFAULTLIB:msvcrt.lib)
    add_definitions(/NODEFAULTLIB:libcd.lib)
    add_definitions(/NODEFAULTLIB:libcmtd.lib)
    add_definitions(/NODEFAULTLIB:msvcrtd.lib)

    OPTION(FORCE_STATIC_VCRT "Force /MT for static VC runtimes" OFF)
    if(FORCE_STATIC_VCRT)
        foreach(flag_var
                CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
                CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
            if(${flag_var} MATCHES "/MD")
                string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
            endif()
        endforeach(flag_var)
    endif()
else(MSVC)
    add_definitions(-Wall -s)
    if(NOT MINGW)
      if (GCC)
        string(APPEND CMAKE_C_FLAGS " -Wl,--hash-style=both")
      endif(GCC)
      if (GCC OR CLANG)
        string(APPEND CMAKE_C_FLAGS " -fPIC -Wpointer-arith -Wparentheses")
      endif(GCC OR CLANG)
    else(NOT MINGW)
       string(APPEND CMAKE_C_FLAGS " -static-libgcc")
       set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -Wl,--out-implib,${LIBZEROXML}.lib")
       foreach(flag_var
                CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
                CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
                CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
                CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
            if(${flag_var} MATCHES "-mthreads")
                string(REGEX REPLACE "-mthreads" "" ${flag_var} "${${flag_var}}")
            endif()
        endforeach(flag_var)
    endif(NOT MINGW)

    set(RELEASE_FLAGS "-O3 -DRELEASE=1")

    # Note: CMAKE_C_FLAGS_RELWITHDEBINFO adds profiler flags
     set(CMAKE_C_FLAGS_RELWITHDEBINFO "-g ${RELEASE_FLAGS} -DNDEBUG" CACHE STRING
         "Flags used by the compiler during Release with Debug Info builds."
         FORCE)
     set(CMAKE_C_FLAGS_MINSIZEREL "-O2 -s -fomit-frame-pointer -DNDEBUG -DRELEASE=1" CACHE STRING
         "Flags used by the compiler during release minsize builds."
         FORCE)
     set(CMAKE_C_FLAGS_RELEASE "${RELEASE_FLAGS} -fomit-frame-pointer -DNDEBUG"
             CACHE STRING "Flags used by the compiler during release builds"
            FORCE)
     set(CMAKE_C_FLAGS_DEBUG "-g3 -D_DEBUG" CACHE STRING
         "Flags used by the compiler during debug builds."
         FORCE)

     set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g ${RELEASE_FLAGS} -DNDEBUG" CACHE STRING
         "Flags used by the compiler during Release with Debug Info builds."
         FORCE)
     set(CMAKE_CXX_FLAGS_MINSIZEREL "-O2 -s -fomit-frame-pointer -DNDEBUG" CACHE STRING
         "Flags used by the compiler during release minsize builds."
         FORCE)
     set(CMAKE_CXX_FLAGS_RELEASE "${RELEASE_FLAGS}"
             CACHE STRING "Flags used by the compiler during release builds"
            FORCE)
     set(CMAKE_CXX_FLAGS_DEBUG "-g3 -D_DEBUG" CACHE STRING
         "Flags used by the compiler during debug builds."
         FORCE)

endif(MSVC)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C flags: ${CMAKE_C_FLAGS}")

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/config.h")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/xml.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/xml.h"
)
set(ZEROXML_OBJS
    src/xml.c
    src/xml_cache.c
    src/localize.c
    src/easyxml.cpp
   )

# Build a library
if(CMAKE_SIZEOF_VOID_P MATCHES "8")
  set(LIB_POSTFIX "64" CACHE STRING "suffix for 32/64 dir placement")
  MARK_AS_ADVANCED(LIB_POSTFIX)
endif(CMAKE_SIZEOF_VOID_P MATCHES "8")
if(NOT DEFINED LIB_POSTFIX)
  set(LIB_POSTFIX "")
endif(NOT DEFINED LIB_POSTFIX)

set(LIB_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
add_LIBRARY(${LIBZEROXML} ${LIBTYPE} ${ZEROXML_OBJS})
set_target_PROPERTIES(${LIBZEROXML}
    PROPERTIES DEFINE_SYMBOL XML_BUILD_LIBRARY
    COMPILE_FLAGS "-D_PROTOTYPES -URMALLOC"
    VERSION ${LIB_VERSION}
    SOVERSION ${CPACK_PACKAGE_VERSION_MAJOR})
if(WIN32 AND NOT LIBTYPE STREQUAL "STATIC")
    set_target_PROPERTIES(${LIBZEROXML} PROPERTIES PREFIX "")
endif()

target_LINK_LIBRARIES(${LIBZEROXML} ${EXTRA_LIBS})

# Add an install target here
install(TARGETS ${LIBZEROXML}
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION "lib${LIB_POSTFIX}"
       )

if(RMALLOC)
  set(ZEROXMLDEBUG_OBJS
      src/xml.c
      src/xml_cache.c
      src/rmalloc.c
     )

  add_LIBRARY(${LIBZEROXML_DEBUG} ${LIBTYPE} ${ZEROXMLDEBUG_OBJS})
  set_target_PROPERTIES(${LIBZEROXML_DEBUG}
                         PROPERTIES DEFINE_SYMBOL XML_BUILD_LIBRARY
                         COMPILE_FLAGS "-D_PROTOTYPES -DMALLOC_DEBUG=ON"
                         VERSION ${LIB_VERSION}
                         SOVERSION ${CPACK_PACKAGE_VERSION_MAJOR})
  if(WIN32 AND NOT LIBTYPE STREQUAL "STATIC")
    set_target_PROPERTIES(${LIBZEROXML_DEBUG} PROPERTIES PREFIX "")
  endif()

  target_LINK_LIBRARIES(${LIBZEROXML_DEBUG} ${EXTRA_LIBS})

  # Add an install target here
  install(TARGETS ${LIBZEROXML_DEBUG}
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION "lib${LIB_POSTFIX}"
       )
endif(RMALLOC)

install(FILES
        include/rmalloc.h
        include/easyxml.hxx
        ${CMAKE_CURRENT_BINARY_DIR}/include/xml.h
        DESTINATION include
       )

add_definitions(-DSOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
add_subdirectory(test)
if(UTILS)
  add_subdirectory(examples)
endif(UTILS)

#-----------------------------------------------------------------------------
### uninstall target
#-----------------------------------------------------------------------------
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)
add_custom_target(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

