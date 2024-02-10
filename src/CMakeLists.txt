#
# Blink
#

cmake_minimum_required(VERSION 3.12)

## configure executable
add_executable(rpi_pico_blink)
target_sources(rpi_pico_blink PRIVATE
    main.c
)
target_include_directories(rpi_pico_blink PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)
target_compile_options(rpi_pico_blink PRIVATE
    -Wall
)
target_link_libraries(rpi_pico_blink
    pico_stdlib
    hardware_spi
)

pico_add_extra_outputs(rpi_pico_blink)
