

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(U1A2 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(
    U1A2 
    U1A2.c 
    lib/config/config.c
    lib/ssd1306/ssd1306.c # Biblioteca para o display OLED
    lib/ws2812/ws2812.c # Biblioteca para a matriz 5x5
    lib/cyw43/cyw43.c
)

pico_set_program_name(U1A2 "U1A2")
pico_set_program_version(U1A2 "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(U1A2 0)
pico_enable_stdio_usb(U1A2 1)

pico_generate_pio_header(${PROJECT_NAME} 
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/ws2812/ws2812.pio 
    OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated
)

# Add the standard library to the build
target_link_libraries(U1A2
        pico_stdlib
        hardware_gpio
        hardware_adc
        hardware_timer
        hardware_pwm
        hardware_i2c
        hardware_pio
        pico_cyw43_arch_lwip_threadsafe_background
)

# Add the standard include files to the build
target_include_directories(U1A2 PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${PICO_SDK_PATH}/lib/lwip/src/include
    ${PICO_SDK_PATH}/lib/lwip/src/include/arch
    ${PICO_SDK_PATH}/lib/lwip/src/include/lwip
)

target_sources(U1A2 PRIVATE
    ${PICO_SDK_PATH}/lib/lwip/src/apps/http/httpd.c
    ${PICO_SDK_PATH}/lib/lwip/src/apps/http/fs.c
)

# Add any user requested libraries

pico_add_extra_outputs(U1A2)
