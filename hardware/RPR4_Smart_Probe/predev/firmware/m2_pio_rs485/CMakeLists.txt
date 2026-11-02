cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(rpr4_m2_pio_rs485 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(rpr4_m2_pio_rs485
    main.c
    ln485_driver.c
)

# PIO ヘッダ自動生成
pico_generate_pio_header(rpr4_m2_pio_rs485 ${CMAKE_CURRENT_LIST_DIR}/ln485.pio)

target_include_directories(rpr4_m2_pio_rs485 PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(rpr4_m2_pio_rs485
    pico_stdlib
    hardware_pio
    hardware_uart
    hardware_clocks
)

# USB CDC で標準出力を有効化 (シリアルモニタで結果確認可能)
pico_enable_stdio_usb(rpr4_m2_pio_rs485 1)
pico_enable_stdio_uart(rpr4_m2_pio_rs485 0)

pico_add_extra_outputs(rpr4_m2_pio_rs485)

