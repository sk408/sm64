cmake_minimum_required(VERSION 3.13)

# Include Pico SDK
include(pico_sdk_import.cmake)

# Project initialization
project(pico_asha VERSION 1.0.0 LANGUAGES C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

# Initialize Pico SDK
pico_sdk_init()

# Set build options
set(CMAKE_BUILD_TYPE Debug)

# Include directories
include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/lib
)

# Add external dependencies
include(FetchContent)

# Fetch Embedded Template Library (ETL)
FetchContent_Declare(
    etl
    GIT_REPOSITORY https://github.com/ETLCPP/etl.git
    GIT_TAG 20.35.11
)
FetchContent_MakeAvailable(etl)
include_directories(${etl_SOURCE_DIR}/include)

# Fetch ArduinoJson for configuration
FetchContent_Declare(
    ArduinoJson
    GIT_REPOSITORY https://github.com/bblanchon/ArduinoJson.git
    GIT_TAG v6.21.3
)
FetchContent_MakeAvailable(ArduinoJson)
include_directories(${ArduinoJson_SOURCE_DIR}/src)

# G.722 codec library
add_subdirectory(lib/g722_codec)

# Main source files
file(GLOB_RECURSE SOURCES 
    "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
)

# Add executable
add_executable(pico_asha ${SOURCES})

# Set target properties
pico_enable_stdio_usb(pico_asha 1)
pico_enable_stdio_uart(pico_asha 0)

# Add compiler options
target_compile_options(pico_asha PRIVATE
    -Wall
    -Wextra
    -Werror
    -O2
)

# Link libraries
target_link_libraries(pico_asha
    pico_stdlib
    pico_multicore
    pico_btstack_ble
    pico_btstack_cyw43
    pico_cyw43_arch_none
    hardware_flash
    hardware_dma
    hardware_irq
    hardware_i2c
    hardware_pwm
    g722_codec
)

# Enable BLE and disable classic bluetooth
target_compile_definitions(pico_asha PRIVATE
    PICO_BTSTACK_CYWBT=1
    BLE_ONLY=1
)

# Create UF2 file for easy flashing
pico_add_extra_outputs(pico_asha)

# Documentation
find_package(Doxygen)
if(DOXYGEN_FOUND)
    add_custom_target(doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif() 