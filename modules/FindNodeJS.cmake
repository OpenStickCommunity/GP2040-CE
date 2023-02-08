# Find the Node.js executable
#
# The following variables are set:
# NODEJS_FOUND      True if Node.js was found
# NODEJS_EXECUTABLE Path to the Node.js executable
# NODEJS_VERSION    Node.js version

find_program(NODEJS_EXECUTABLE NAMES node nodejs)

if (${NODEJS_EXECUTABLE} MATCHES "NOTFOUND")
  message(STATUS "Could not find Node.js")
  set(NODEJS_FOUND FALSE CACHE INTERNAL "")
  unset(NODEJS_EXECUTABLE)
else()
  execute_process(COMMAND ${NODEJS_EXECUTABLE} --version
    OUTPUT_VARIABLE VERSION_VAR
    ERROR_VARIABLE ERROR_VAR
    RESULT_VARIABLE RESULT_VAR)

  if (RESULT_VAR EQUAL 0)
    string(REGEX REPLACE "[v]" "" VERSION_VAR ${VERSION_VAR})
    string(STRIP ${VERSION_VAR} VERSION_VAR)
    set(NODEJS_VERSION ${VERSION_VAR} CACHE INTERNAL "")
    set(NODEJS_FOUND TRUE CACHE INTERNAL "")
    message(STATUS "Found Node.js version ${NODEJS_VERSION}: ${NODEJS_EXECUTABLE}")
  else()
    message(WARNING "Failed to execute NodeJS --version command: ${ERROR_VAR}")
  endif()
endif()