cmake_minimum_required(VERSION 3.12)

set(CMAKE_BUILD_TYPE RelWithDebInfo)

include(pico_sdk_import.cmake)

set(FREERTOS_KERNEL_PATH ${CMAKE_CURRENT_LIST_DIR}/freertos)
include(FreeRTOS_Kernel_import.cmake)

project(debugprobe)

pico_sdk_init()

if (${PICO_SDK_VERSION_MAJOR} LESS 2)
	message(SEND_ERROR "Version 2 of the Pico SDK is required to compile this project. Please update your installation at ${PICO_SDK_PATH}")
endif ()

set(DEBUGPROBE_SOURCES
        src/probe_config.c
        src/main.c
        src/usb_descriptors.c
        src/probe.c
        src/cdc_uart.c
        src/get_serial.c
        src/sw_dp_pio.c
        src/tusb_edpt_handler.c
        src/autobaud.c
        src/ws2812_led.c
)

set(CMSIS_DAP_SOURCES
        CMSIS_DAP/CMSIS/DAP/Firmware/Source/DAP.c
        CMSIS_DAP/CMSIS/DAP/Firmware/Source/JTAG_DP.c
        CMSIS_DAP/CMSIS/DAP/Firmware/Source/DAP_vendor.c
        CMSIS_DAP/CMSIS/DAP/Firmware/Source/SWO.c
        #CMSIS_DAP/CMSIS/DAP/Firmware/Source/SW_DP.c
)

set(DEBUGPROBE_PIO_DIR ${CMAKE_CURRENT_LIST_DIR}/src)

# add version
add_custom_target(version
        ${CMAKE_SOURCE_DIR}/get-version.sh
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

function(add_debugprobe_target name)
    add_executable(${name} ${DEBUGPROBE_SOURCES})
    target_sources(${name} PRIVATE ${CMSIS_DAP_SOURCES})
    target_include_directories(${name} PRIVATE
            CMSIS_DAP/CMSIS/DAP/Firmware/Include/
            CMSIS_DAP/CMSIS/Core/Include/
            include/
            src
            ${CMAKE_BINARY_DIR}/generated
    )
    target_compile_options(${name} PRIVATE -Wall)
    pico_generate_pio_header(${name} ${DEBUGPROBE_PIO_DIR}/probe.pio)
    pico_generate_pio_header(${name} ${DEBUGPROBE_PIO_DIR}/probe_oen.pio)
    pico_generate_pio_header(${name} ${DEBUGPROBE_PIO_DIR}/autobaud.pio)
    pico_generate_pio_header(${name} ${DEBUGPROBE_PIO_DIR}/ws2812_led.pio)
    add_dependencies(${name} version)
    target_compile_definitions(${name} PRIVATE
	PICO_RP2040_USB_DEVICE_ENUMERATION_FIX=1
    )
    target_link_libraries(${name} PRIVATE
            pico_multicore
            pico_stdlib
            pico_unique_id
            tinyusb_device
            tinyusb_board
            hardware_pio
            hardware_dma
            hardware_irq
            hardware_clocks
            FreeRTOS-Kernel
            FreeRTOS-Kernel-Heap1
    )
    pico_set_binary_type(${name} copy_to_ram)
    pico_add_extra_outputs(${name})
endfunction()

add_debugprobe_target(debugprobe)

option (DEBUG_ON_PICO "Compile firmware for the Pico instead of Debug Probe" OFF)
if (DEBUG_ON_PICO)
    target_compile_definitions (debugprobe PRIVATE
	DEBUG_ON_PICO=1
    )

    # 4-pin variant with nRESET
    add_debugprobe_target(debugprobe_4pin)
    target_compile_definitions (debugprobe_4pin PRIVATE
	DEBUG_ON_PICO=1
	PROBE_4PIN=1
    )

    if (PICO_BOARD STREQUAL "pico")
        set_target_properties(debugprobe PROPERTIES
            OUTPUT_NAME "debugprobe-zero"
        )
        set_target_properties(debugprobe_4pin PROPERTIES
            OUTPUT_NAME "debugprobe-zero-4pin"
        )
    elseif (PICO_BOARD STREQUAL "pico2")
        set_target_properties(debugprobe PROPERTIES
            OUTPUT_NAME "debugprobe-zero2"
        )
        set_target_properties(debugprobe_4pin PROPERTIES
            OUTPUT_NAME "debugprobe-zero2-4pin"
        )
    else ()
        message(SEND_ERROR "Unsupported board ${PICO_BOARD}")
    endif ()
endif ()
