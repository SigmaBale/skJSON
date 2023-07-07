# Libraries required to be linked to test executables
set(TEST_LIBS ${CMAKE_SOURCE_DIR}/build/lib${DEBUG_LIB}.so criterion)
# Source files
set(TEST_SRCFILES skjson_test.c)
# Test standard
set(TEST_STANDARD -std=c99)

# Helper function for creating test executables
function(create_test)
    set(options)
    set(args TARGET NAME)
    set(list_args SRCFILES LIBS CFLAGS LFLAGS)
    cmake_parse_arguments(
        PARSE_ARGV 0
        test
        "${options}"
        "${args}"
        "${list_args}"
    )
    add_executable("${test_TARGET}" ${test_SRCFILES})
    target_link_libraries("${test_TARGET}" 
        PRIVATE 
        ${test_LIBS})
    target_compile_options("${test_TARGET}" 
        PRIVATE 
        ${WARNING_FLAGS} 
        ${TEST_STANDARD} 
        ${test_CFLAGS})
    target_link_options("${test_TARGET}" 
        PRIVATE 
        ${test_LFLAGS})
    add_test("${test_NAME}" "${test_TARGET}")
endfunction()

# Test executable
create_test(
    TARGET skjson_test 
    NAME skjson
    SRCFILES ${TEST_SRCFILES}
    LIBS ${TEST_LIBS}
)

# Test Input files
configure_file(error.json ${CMAKE_BINARY_DIR}/tests/error.json COPYONLY)
configure_file(simple.json ${CMAKE_BINARY_DIR}/tests/simple.json COPYONLY)
configure_file(meta_github.json ${CMAKE_BINARY_DIR}/tests/meta_github.json COPYONLY)
