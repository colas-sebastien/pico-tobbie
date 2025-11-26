cmake_minimum_required(VERSION 3.13)

# PICO_BOARD should be set to 'pico_w', 'pico2_w' or not set at all.
if(DEFINED PICO_BOARD)
    message(DEBUG, "Using PICO_BOARD ${PICO_BOARD}")
else()
    message(VERBOSE, "PICO_BOARD not defined. Using 'pico2_w'")
    set(PICO_BOARD "pico2_w")
endif()

# BLUEPAD32_ROOT must be defined.
if (DEFINED ENV{BLUEPAD32_ROOT} AND (NOT BLUEPAD32_ROOT))
    set(BLUEPAD32_ROOT $ENV{BLUEPAD32_ROOT})
    message("Using BLUEPAD32_ROOT from environment ('${BLUEPAD32_ROOT}')")
endif ()
if(DEFINED BLUEPAD32_ROOT)
    message(DEBUG, "Using BLUEPAD32_ROOT ${BLUEPAD32_ROOT}")
else()  
    message( FATAL_ERROR "BLUEPAD32_ROOT must be defined (https://github.com/ricardoquesada/bluepad32)" )    
endif()

# To use BTstack from Pico SDK do
set(BTSTACK_ROOT ${PICO_SDK_PATH}/lib/btstack)

# Change your executable name to something creative!
set(NAME pico-tobbie) # <-- Name your project/executable here!

include(pico_sdk_import.cmake)
include(pimoroni_pico_import.cmake)

# Gooey boilerplate
project(${NAME} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

# Add your source files
add_executable(${NAME}
    src/main.cpp # <-- Add source files here!
    src/pico_tobbie.cpp
    ${BTSTACK_MISSING_SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    src
    ${BLUEPAD32_ROOT}/src/components/bluepad32/include)

# Include required libraries
# This assumes `pimoroni-pico` is stored alongside your project
#include(common/pimoroni_i2c)
#include(common/pimoroni_bus)
#include(libraries/bitmap_fonts/bitmap_fonts)
#include(libraries/hershey_fonts/hershey_fonts)
#include(libraries/pico_explorer/pico_explorer)
include(libraries/pico_motor_shim/pico_motor_shim)

# Needed for btstack_config.h / sdkconfig.h
# so that libbluepad32 can include them
include_directories(${PROJECT_NAME} src)

# Needed when using BTstack from our branch
include_directories(${BTSTACK_ROOT}/3rd-party/bluedroid/encoder/include)
include_directories(${BTSTACK_ROOT}/3rd-party/bluedroid/decoder/include)
# Need for certain IDEs, like CLion. Otherwise it won't find btstack
include_directories(${BTSTACK_ROOT}/src)

# Don't forget to link the libraries you need!
target_link_libraries(${NAME}
    pico_stdlib
    pico_motor_shim
    pico_cyw43_arch_none
    pico_btstack_classic
    pico_btstack_cyw43
    bluepad32    
)

add_subdirectory(${BLUEPAD32_ROOT}/src/components/bluepad32 libbluepad32)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(${NAME})

# Set up files for the release packages
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.uf2
    ${CMAKE_CURRENT_LIST_DIR}/README.md
    DESTINATION .
)

set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_GENERATOR "ZIP" "TGZ")
include(CPack)
