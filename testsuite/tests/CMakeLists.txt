project(userver-testsuite-test)

add_test (NAME ${PROJECT_NAME}
  COMMAND ${TESTSUITE_PYTHON_BINARY} -m pytest ${CMAKE_CURRENT_SOURCE_DIR}
  WORKING_DIRECTORY "${USERVER_TESTSUITE_DIR}/pytest_plugins"
)

set_tests_properties(${PROJECT_NAME} PROPERTIES TIMEOUT 60) 

add_custom_target(
  run-${PROJECT_NAME}
  COMMAND ${TESTSUITE_PYTHON_BINARY} -m pytest ${CMAKE_CURRENT_SOURCE_DIR}
  WORKING_DIRECTORY "${USERVER_TESTSUITE_DIR}/pytest_plugins"
)
