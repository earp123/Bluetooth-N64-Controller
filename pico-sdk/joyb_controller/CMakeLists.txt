# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
cmake_minimum_required(VERSION 3.15)

# The name of our project
set(PROJECT_NAME joyb_controller)

# The board name. Use one of the header file names from "${PICO_SDK_PATH}/src/boards/include/boards"
# Official Pico boards are:
# pico, pico_w, pico2, pico2_w
set(PICO_BOARD pico_w)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

project(${PROJECT_NAME} C CXX)
set(TARGET_NAME ${PROJECT_NAME})


add_executable(${TARGET_NAME}
    joyb_controller.c
    myBLEsrvc.c
)

pico_btstack_make_gatt_header(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/ble_tx_u32.gatt)


target_compile_definitions(${TARGET_NAME} PRIVATE
)



target_link_libraries(${TARGET_NAME} 
    pico_stdlib
    hardware_pio
    pico_multicore
    pico_cyw43_arch_none       # or _poll / _threadsafe_background
    pico_btstack_cyw43
    pico_btstack_ble
)

target_include_directories(${TARGET_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/..
    ${CMAKE_CURRENT_LIST_DIR}/../..
)

pico_add_extra_outputs(${TARGET_NAME})

pico_enable_stdio_uart(${TARGET_NAME} 1)
pico_enable_stdio_usb(${TARGET_NAME} 1)

pico_generate_pio_header(${TARGET_NAME} ${CMAKE_CURRENT_LIST_DIR}/joybus.pio)
pico_generate_pio_header(${TARGET_NAME} ${CMAKE_CURRENT_LIST_DIR}/pulse_to_byte.pio)

