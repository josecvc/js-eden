cmake_minimum_required(VERSION 3.16)
project(DSP CXX)

add_definitions(-std=c++17)
set(CMAKE_CXX_STANDARD 17)

add_executable(dsp src/dsp.cpp)

set_target_properties(dsp PROPERTIES SUFFIX ".wasm")

target_link_options(dsp PRIVATE
    "--no-entry"
    "-sSTANDALONE_WASM"
    "-sEXPORTED_FUNCTIONS=['_init_wavetable','_process', '_malloc', '_free']"
    "-sERROR_ON_UNDEFINED_SYMBOLS=0"
    "-O3"
    "-msimd128"
    "-sALLOW_MEMORY_GROWTH=0"
    "-sINITIAL_MEMORY=16MB"
)
