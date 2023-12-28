cmake_minimum_required(VERSION 3.13)
include(CMakePrintHelpers)




# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.5.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.5.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# set the version for webconfig, etc. based on git
find_package(Git)
execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags --always --dirty
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	OUTPUT_VARIABLE GIT_REPO_VERSION
	OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE "v([0-9]+\\.[0-9]+\\.[0-9]+).*" "\\1" CMAKE_GIT_REPO_VERSION ${GIT_REPO_VERSION})
string(REGEX REPLACE "^.......$" "0.0.0" CMAKE_GIT_REPO_VERSION ${CMAKE_GIT_REPO_VERSION}) # fix if all we have is the git SHA
configure_file("headers/version.h.in" "headers/version.h")
message("GIT_REPO_VERSION is ${GIT_REPO_VERSION}")
message("CMAKE_GIT_REPO_VERSION is ${CMAKE_GIT_REPO_VERSION}")

# Uncomment the next line for an unomptimized build for debugging. Use in conjunction with the Debug build type.
# set(PICO_DEOPTIMIZED_DEBUG 1)

project(GP2040-CE LANGUAGES C CXX ASM VERSION ${CMAKE_GIT_REPO_VERSION})
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

if(DEFINED ENV{PICO_PIO_USB_PATH})
  message(STATUS "Found custom Pico-PIO-USB path, using it.")
  set(PICO_PIO_USB_PATH $ENV{PICO_PIO_USB_PATH})
elseif(NOT DEFINED PICO_PIO_USB_PATH)
  message(STATUS "Using default Pico-PIO-USB.")
  set(PICO_PIO_USB_PATH ${CMAKE_SOURCE_DIR}/lib/pico_pio_usb)
endif()

add_compile_options(-Wall
        -Wtype-limits
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        )

include(compile_proto.cmake)
compile_proto()

#pull in tinyUSB
set(PICO_TINYUSB_PATH "${CMAKE_CURRENT_LIST_DIR}/lib/tinyusb")

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# Remove unused code and data
add_compile_options(-fdata-sections -ffunction-sections)
add_link_options(-Wl,--gc-sections)

add_subdirectory(lib)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  # Activate some compiler / linker options to aid us with diagnosing stack space issues in Debug builds
  add_compile_options(-fstack-usage -Wstack-usage=500)
  add_compile_definitions(PICO_USE_STACK_GUARDS=1)
endif()

# We want a larger stack of 4kb per core instead of the default 2kb
add_compile_definitions(PICO_STACK_SIZE=0x1000)

add_executable(${PROJECT_NAME}
src/main.cpp
src/gp2040.cpp
src/gp2040aux.cpp
src/gamepad.cpp
src/addonmanager.cpp
src/configmanager.cpp
src/peripheralmanager.cpp
src/storagemanager.cpp
src/system.cpp
src/usbhostmanager.cpp
src/config_legacy.cpp
src/config_utils.cpp
src/configs/webconfig.cpp
src/addons/analog.cpp
src/addons/board_led.cpp
src/addons/bootsel_button.cpp
src/addons/focus_mode.cpp
src/addons/buzzerspeaker.cpp
src/addons/dualdirectional.cpp
src/addons/keyboard_host.cpp
src/addons/i2canalog1219.cpp
src/addons/jslider.cpp
src/addons/i2cdisplay.cpp
src/addons/neopicoleds.cpp
src/addons/playernum.cpp
src/addons/playerleds.cpp
src/addons/ps4mode.cpp
src/addons/pspassthrough.cpp
src/addons/reverse.cpp
src/addons/turbo.cpp
src/addons/slider_socd.cpp
src/addons/wiiext.cpp
src/addons/input_macro.cpp
src/addons/snes_input.cpp
src/addons/inputhistory.cpp
src/gamepad/GamepadDebouncer.cpp
src/gamepad/GamepadDescriptors.cpp
src/addons/tilt.cpp
src/addons/xbonepassthrough.cpp
${PROTO_OUTPUT_DIR}/enums.pb.c
${PROTO_OUTPUT_DIR}/config.pb.c
)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME}_${CMAKE_PROJECT_VERSION}_${GP2040_BOARDCONFIG})

pico_set_program_name(GP2040-CE "GP2040-CE")
pico_set_program_version(GP2040-CE ${GIT_REPO_VERSION})

target_link_libraries(${PROJECT_NAME}
pico_stdlib
pico_bootsel_via_double_reset
tinyusb_host
tinyusb_pico_pio_usb
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
PicoPeripherals
WiiExtension
SNESpad
pico_mbedtls
TinyUSB_Gamepad
nanopb
)

target_include_directories(${PROJECT_NAME} PUBLIC
headers
headers/addons
headers/configs
headers/gamepad
configs/${GP2040_BOARDCONFIG}
${PROTO_OUTPUT_DIR}
${CMAKE_BINARY_DIR}/headers
)

target_compile_definitions(${PROJECT_NAME} PUBLIC
  PICO_XOSC_STARTUP_DELAY_MULTIPLIER=64
  BOARD_CONFIG_FILE_NAME="$<TARGET_FILE_BASE_NAME:${PROJECT_NAME}>"
  GP2040_BOARDCONFIG="${GP2040_BOARDCONFIG}"
)

target_include_directories(${PROJECT_NAME}  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
  )

pico_add_extra_outputs(${PROJECT_NAME})

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_${CMAKE_PROJECT_VERSION}_${GP2040_BOARDCONFIG}.uf2
    ${CMAKE_CURRENT_LIST_DIR}/README.md
    DESTINATION .
)

if (NOT (DEFINED ENV(CI)) AND (EXISTS ${CMAKE_SOURCE_DIR}/modules/Custom.cmake))
  message(STATUS "Found custom script.")
	include(${CMAKE_SOURCE_DIR}/modules/Custom.cmake)
endif()
