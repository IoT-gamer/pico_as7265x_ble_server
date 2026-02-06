# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico_as7265x_ble C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

include(FetchContent)

FetchContent_Declare(
    as7265x_pico 
    GIT_REPOSITORY https://github.com/IoT-gamer/pico-as7265x.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(as7265x_pico)

# Add executable. Default name is the project name, version 0.1

add_executable(pico_as7265x_ble
    main.c 
    ble_server.c
)

pico_set_program_name(pico_as7265x_ble "pico_as7265x_ble")
pico_set_program_version(pico_as7265x_ble "0.1")

# Process .gatt file into a C header
pico_btstack_make_gatt_header(pico_as7265x_ble PRIVATE "${CMAKE_CURRENT_LIST_DIR}/spectral.gatt")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(pico_as7265x_ble 0)
pico_enable_stdio_usb(pico_as7265x_ble 1)

target_link_libraries(pico_as7265x_ble PRIVATE
    pico_stdlib
    pico_cyw43_arch_none # Use 'none' if you don't need WiFi, saves RAM
    pico_btstack_ble
    pico_btstack_cyw43
    hardware_i2c
    as7265x_pico
)

# Add the standard include files to the build
target_include_directories(pico_as7265x_ble PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)


pico_add_extra_outputs(pico_as7265x_ble)

