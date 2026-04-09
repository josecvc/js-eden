cmake_minimum_required(VERSION 3.16)
project(DSP CXX)

add_definitions(-std=c++17)
set(CMAKE_CXX_STANDARD 17)

add_executable(dsp 
    src/wavetable.cpp
    src/polyphony.cpp
    src/sample.cpp
    src/instrument.cpp
    src/miniaudio.cpp
    src/pattern.cpp
    src/editing.cpp
    src/engine.cpp
)

set_target_properties(dsp PROPERTIES SUFFIX ".wasm")

target_compile_options(dsp PRIVATE
    "-flto"
)

target_link_options(dsp PRIVATE
    "--no-entry"
    "-sSTANDALONE_WASM=1"
    "-sEXPORTED_FUNCTIONS=['_malloc', '_free', '_create_engine', '_destroy_engine', '_init_engine', '_process','_register_sample', '_clear_sample','_play_from_midi', '_play_sample','_stop_sample','_play_track','_pause_track','_stop_track','_set_tempo','_set_sample_rate','_set_ticks_per_row','_get_num_voices','_get_instrument_id_from_channel','_get_note_id_from_channel','_get_current_row', '_get_current_pattern','_get_current_order','_get_playback_state' ,'_stop_from_midi', '_get_first_cell', '_set_note', '_set_instrument', '_set_volume', '_set_effect', '_set_param', '_get_bpm','_insert_order','_remove_order','_increase_channel_count','_decrease_channel_count','_increase_row_count','_decrease_row_count','_expand_pattern','_shrink_pattern','_set_instrument_sample','_set_instrument_synth','_get_sample_playhead_ptr','_get_envelope_playhead_ptr', '_get_num_rows', '_cut_sample', '_copy_sample', '_paste_sample', '_crop_sample', '_reverse_sample', '_get_bins_from_sample', '_get_sample_length', '_get_sample_loop_from', '_get_sample_loop_to', '_set_view', '_set_loop_type', '_set_loop_from', '_set_loop_to', '_add_envelope_point', '_delete_envelope_point', '_set_envelope_point_value', '_enable_envelope', '_disable_envelope', '_enable_envelope_sustain', '_disable_envelope_sustain','_enable_envelope_loop', '_disable_envelope_loop','_set_envelope_sustain', '_set_envelope_loop_from', '_set_envelope_loop_to', '_set_instrument_note_sample']"
    "-sERROR_ON_UNDEFINED_SYMBOLS=0"
    "-O3"
    "-msimd128"
    "-sALLOW_MEMORY_GROWTH=0"
    "-sINITIAL_MEMORY=512MB"
    "-flto"
)
