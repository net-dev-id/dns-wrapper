cmake_minimum_required(VERSION 3.14)

# List all files containing tests. (Change as needed)
set(TESTFILES # All .cpp files in tests/
    main.cpp
    dummy.cpp
)

set(TEST_MAIN unit_tests) # Default name for test executable (change if you wish).
set(TEST_RUNNER_PARAMS "") # Any arguemnts to feed the test runner (change as needed).
add_executable(${TEST_MAIN} ${TESTFILES})
target_link_libraries(${TEST_MAIN} PRIVATE ${LIBRARY_NAME} doctest -lboost_program_options -lboost_log -lboost_thread -lboost_log_setup -lpthread -lboost_system)
set_target_properties(${TEST_MAIN} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
target_set_warnings(${TEST_MAIN} ENABLE ALL AS_ERROR ALL DISABLE Annoying) # Set warnings (if needed).
add_test(
    # Use some per-module/project prefix so that it is easier to run only tests for this module
    NAME ${LIBRARY_NAME}.${TEST_MAIN}
    COMMAND ${TEST_MAIN} ${TEST_RUNNER_PARAMS})

# Adds a 'coverage' target.
include(CodeCoverage)
