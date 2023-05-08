cmake_minimum_required(VERSION 3.13)
include(CMakePrintHelpers)




# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.5.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.5.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(GP2040-CE LANGUAGES C CXX ASM VERSION 0.7.1)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if(DEFINED ENV{GP2040_BOARDCONFIG})
  set(GP2040_BOARDCONFIG $ENV{GP2040_BOARDCONFIG})
elseif(NOT DEFINED GP2040_BOARDCONFIG)
  set(GP2040_BOARDCONFIG Pico)
endif()

if(DEFINED ENV{SKIP_SUBMODULES})
  set(SKIP_SUBMODULES $ENV{SKIP_SUBMODULES})
elseif(NOT DEFINED SKIP_SUBMODULES)
  set(SKIP_SUBMODULES FALSE)
endif()

if(DEFINED ENV{SKIP_WEBBUILD})
  set(SKIP_WEBBUILD $ENV{SKIP_WEBBUILD})
elseif(NOT DEFINED SKIP_WEBBUILD)
  set(SKIP_WEBBUILD FALSE)
endif()


if(SKIP_SUBMODULES)
  cmake_print_variables(SKIP_SUBMODULES)
else()
  find_package(Git)
  if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
      execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                      RESULT_VARIABLE GIT_SUBMOD_RESULT)
      if(NOT GIT_SUBMOD_RESULT EQUAL "0")
          message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
      else()
          message("Submodules updated")
      endif()
  endif()
endif()

if(SKIP_WEBBUILD)
  cmake_print_variables(SKIP_WEBBUILD)
else()
  message(STATUS "Not Skipping WebBuild")
  include(${CMAKE_SOURCE_DIR}/modules/FindNodeJS.cmake)
  include(${CMAKE_SOURCE_DIR}/modules/FindNPM.cmake)
  if(NODEJS_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/www")
    if(NPM_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/www/package.json")
      execute_process(COMMAND ${NPM_EXECUTABLE} ci
                      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/www
                      RESULT_VARIABLE NPM_CI_RESULT)
      if(NOT NPM_CI_RESULT EQUAL "0")
        message(FATAL_ERROR "npm ci failed with ${NPM_CI_RESULT}")
      endif()
      execute_process(COMMAND ${NPM_EXECUTABLE} run build
                      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/www
                      RESULT_VARIABLE NPM_BUILD_RESULT)
      if(NOT NPM_BUILD_RESULT EQUAL "0")
        message(FATAL_ERROR "npm run build failed with ${NPM_BUILD_RESULT}")
      endif()
    endif()
  endif()
endif()


set(PICO_BOARD_HEADER_DIRS ${CMAKE_SOURCE_DIR}/configs/${GP2040_BOARDCONFIG})
include(FetchContent)
FetchContent_Declare(ArduinoJson
    GIT_REPOSITORY https://github.com/bblanchon/ArduinoJson.git
    GIT_TAG        v6.21.2
)
FetchContent_MakeAvailable(ArduinoJson)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()
add_subdirectory(lib)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  # Activate some compiler / linker options to aid us with diagnosing stack space issues in Debug builds
  add_compile_options(-fstack-usage)
  add_compile_definitions(PICO_USE_STACK_GUARDS=1)
endif()

# We want a larger stack of 4kb per core instead of the default 2kb
add_compile_definitions(PICO_STACK_SIZE=0x1000)

add_compile_options(-Wstack-usage=500)

add_executable(${PROJECT_NAME}
src/main.cpp
src/gp2040.cpp
src/gp2040aux.cpp
src/gamepad.cpp
src/addonmanager.cpp
src/configmanager.cpp
src/storagemanager.cpp
src/system.cpp
src/configs/webconfig.cpp
src/addons/analog.cpp
src/addons/board_led.cpp
src/addons/bootsel_button.cpp
src/addons/buzzerspeaker.cpp
src/addons/dualdirectional.cpp
src/addons/extra_button.cpp
src/addons/i2canalog1219.cpp
src/addons/jslider.cpp
src/addons/i2cdisplay.cpp
src/addons/neopicoleds.cpp
src/addons/playernum.cpp
src/addons/playerleds.cpp
src/addons/ps4mode.cpp
src/addons/reverse.cpp
src/addons/turbo.cpp
src/addons/slider_socd.cpp
src/addons/wiiext.cpp
src/gamepad/GamepadDebouncer.cpp
src/gamepad/GamepadDescriptors.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME}_${CMAKE_PROJECT_VERSION}_${GP2040_BOARDCONFIG})

pico_set_program_name(GP2040-CE "GP2040-CE")
pico_set_program_version(GP2040-CE "0.7.1")

target_link_libraries(${PROJECT_NAME}
pico_stdlib
pico_bootsel_via_double_reset
AnimationStation
CRC32
FlashPROM
ADS1219
PlayerLEDs
NeoPico
OneBitDisplay
ArduinoJson
rndis
hardware_adc
WiiExtension
pico_mbedtls
TinyUSB_Gamepad
)

target_include_directories(${PROJECT_NAME} PUBLIC
headers
headers/addons
headers/configs
headers/gamepad
configs/${GP2040_BOARDCONFIG}
)

target_compile_definitions(${PROJECT_NAME} PUBLIC
  PICO_XOSC_STARTUP_DELAY_MULTIPLIER=64
)

target_include_directories(${PROJECT_NAME}  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
  )

pico_add_extra_outputs(${PROJECT_NAME})

add_compile_options(-Wall
        -Wtype-limits
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )

        install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_${CMAKE_PROJECT_VERSION}_${GP2040_BOARDCONFIG}.uf2
    ${CMAKE_CURRENT_LIST_DIR}/README.md
    DESTINATION .
)

if (NOT (DEFINED ENV(CI)) AND (EXISTS ${CMAKE_SOURCE_DIR}/modules/Custom.cmake))
  message(STATUS "Found custom script.")
	include(${CMAKE_SOURCE_DIR}/modules/Custom.cmake)
endif()