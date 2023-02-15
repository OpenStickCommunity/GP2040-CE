# Find the Node.js executable
#
# The following variables are set:
# NPM_FOUND      True if npm was found
# NPM_EXECUTABLE Path to the npm executable
# NPM_VERSION    npm version

find_program(NPM_EXECUTABLE NAMES npm)

if (${NPM_EXECUTABLE} MATCHES "NOTFOUND")
  message(STATUS "Could not find npm")
  set(NPM_FOUND FALSE CACHE INTERNAL "")
  unset(NPM_EXECUTABLE)
else()
  if(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
    set(NPM_EXECUTABLE ${NPM_EXECUTABLE}.cmd)
  endif()
  execute_process(COMMAND ${NPM_EXECUTABLE} --version
    OUTPUT_VARIABLE VERSION_VAR
    ERROR_VARIABLE ERROR_VAR
    RESULT_VARIABLE RESULT_VAR)
  if (RESULT_VAR EQUAL 0)
    string(REGEX REPLACE "[v]" "" VERSION_VAR ${VERSION_VAR})
    string(STRIP ${VERSION_VAR} VERSION_VAR)
    set(NPM_VERSION ${VERSION_VAR} CACHE INTERNAL "")
    set(NPM_FOUND TRUE CACHE INTERNAL "")
    message(STATUS "Found npm version ${NPM_VERSION}: ${NPM_EXECUTABLE}")
  else()
    message(WARNING "Failed to execute npm --version command: ${ERROR_VAR}")
  endif()
endif()