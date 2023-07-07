cmake_minimum_required(VERSION 3.26)

# Project name and version
project(skJSON VERSION 1.0.0)

# Release library name
set(RELEASE_LIB skjson)

# Debug library name
set(DEBUG_LIB skjsondbg)

# Source files directory for shared library
set(SRCDIR ${CMAKE_SOURCE_DIR}/src)

# List of warning flags
set(WARNING_FLAGS -Wall -Werror -Wextra -Wpedantic)

# Source files for shared library
set(SRCFILES
    ${SRCDIR}/skjson.c
    ${SRCDIR}/sknode.c
    ${SRCDIR}/skparser.c
    ${SRCDIR}/skscanner.c
    ${SRCDIR}/skslice.c
    ${SRCDIR}/skutils.c
    ${SRCDIR}/skvec.c)

# Helper function for creating a release/debug shared libraries
function(add_so so_name)
    add_library(${so_name} SHARED ${SRCFILES})
    target_compile_options(${so_name} PRIVATE ${WARNING_FLAGS} -ansi ${ARGN})
    target_include_directories(${so_name} PUBLIC ${SRCDIR})
endfunction()

# Shared library (RELEASE)
add_so(${RELEASE_LIB} -Os)
# Shared library (DEBUG)
add_so(${DEBUG_LIB} -g)
target_compile_definitions(skjsondbg PRIVATE SK_DBUG SK_ERRMSG)

# Install library
install(TARGETS skjson DESTINATION lib)
# Install header file
install(FILES "${SRCDIR}/skjson.h" DESTINATION include)

# Disable building tests by default
if(DEFINED SKJSON_BUILD_TESTS)
    include(CTest)
    add_subdirectory(${CMAKE_SOURCE_DIR}/tests)
    unset(SKJSON_BUILD_TESTS CACHE)
endif()
