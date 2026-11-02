cmake_minimum_required(VERSION 3.13)

# Pico SDK 初期化
include(pico_sdk_import.cmake)

project(rpr4_m1_usb_midi C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(rpr4_m1_usb_midi
    main.c
    usb_descriptors.c
    sysex_protocol.c
)

# インクルードディレクトリ設定
target_include_directories(rpr4_m1_usb_midi PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# ライブラリリンク (TinyUSB device を明示的にリンク)
target_link_libraries(rpr4_m1_usb_midi
    pico_stdlib
    tinyusb_device
)

# USB CDC stdio は無効化 (自前で TinyUSB MIDI をハンドリングするため)
pico_enable_stdio_usb(rpr4_m1_usb_midi 0)
pico_enable_stdio_uart(rpr4_m1_usb_midi 0)

# UF2 および各種バイナリ出力
pico_add_extra_outputs(rpr4_m1_usb_midi)

