add_executable(runEclipseStateTests EclipseStateTests.cpp)
target_link_libraries(runEclipseStateTests Parser ${Boost_LIBRARIES})
add_test(NAME runEclipseStateTests WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} COMMAND ${TEST_MEMCHECK_TOOL} ${EXECUTABLE_OUTPUT_PATH}/runEclipseStateTests )
