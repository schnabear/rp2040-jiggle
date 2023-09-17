cmake_minimum_required(VERSION 3.13)

if (NOT PICO_SDK_PATH)
    if(DEFINED ENV{PICO_SDK_PATH})
        set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
    else()
        set(PICO_SDK_PATH ${CMAKE_CURRENT_LIST_DIR}/pico-sdk)
    endif()
endif()

include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(puffpuff
        DESCRIPTION "AHHH"
        VERSION 0.1.0
        HOMEPAGE_URL https://schnabear.github.io
        )

pico_sdk_init()

add_executable(${PROJECT_NAME})

pico_set_program_name(${PROJECT_NAME} ${PROJECT_NAME})
pico_set_program_description(${PROJECT_NAME} ${CMAKE_PROJECT_DESCRIPTION})
pico_set_program_version(${PROJECT_NAME} ${CMAKE_PROJECT_VERSION})
pico_set_program_url(${PROJECT_NAME} ${CMAKE_PROJECT_HOMEPAGE_URL})

pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/src/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/src/)

target_sources(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src/main.c
        ${CMAKE_CURRENT_LIST_DIR}/src/usb_descriptors.c
        )

# Make sure TinyUSB can find tusb_config.h
target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src
        )

# In addition to pico_stdlib required for common PicoSDK functionality, add dependency on tinyusb_device
# for TinyUSB device support and tinyusb_board for the additional board support library used by the example
# Currently pico_unique_id is unused by TinyUSB 0.15.0
target_link_libraries(${PROJECT_NAME} PRIVATE pico_stdlib tinyusb_device tinyusb_board hardware_pio)

# Uncomment this line to enable fix for Errata RP2040-E5 (the fix requires use of GPIO 15)
#target_compile_definitions(${PROJECT_NAME} PRIVATE PICO_RP2040_USB_DEVICE_ENUMERATION_FIX=1)

pico_add_extra_outputs(${PROJECT_NAME})
