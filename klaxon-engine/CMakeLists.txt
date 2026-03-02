cmake_minimum_required(VERSION 3.16)
project(DSP CXX)

add_definitions(-std=c++17)
set(CMAKE_CXX_STANDARD 17)

add_executable(dsp 
    src/wavetable.cpp
    src/polyphony.cpp
    src/sample.cpp
    src/instrument.cpp
    src/engine.cpp
    src/miniaudio.cpp
)

set_target_properties(dsp PROPERTIES SUFFIX ".wasm")

target_link_options(dsp PRIVATE
    "--no-entry"
    "-sSTANDALONE_WASM"
    "-sEXPORTED_FUNCTIONS=
    [
        '_malloc', 
        '_free', 
        '_create_engine', 
        '_destroy_engine', 
        '_init_engine', 
        '_process',
        '_add_sample',
        '_play_from_midi'
    ]"
    "-sERROR_ON_UNDEFINED_SYMBOLS=0"
    "-O3"
    "-msimd128"
    "-sALLOW_MEMORY_GROWTH=0"
    "-sINITIAL_MEMORY=64MB"
)
