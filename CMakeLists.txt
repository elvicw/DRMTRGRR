cmake_minimum_required(VERSION 3.22)

project(DRMTRGRR VERSION 1.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# If libs/JUCE exists locally, use it; otherwise fetch from GitHub
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/JUCE/CMakeLists.txt")
    add_subdirectory(libs/JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 7.0.12
    )
    FetchContent_MakeAvailable(JUCE)
endif()

# Plugin formats. macOS (AU) is intentionally skipped for this release; Windows VST3 + Standalone only.
set(PLUGIN_FORMATS VST3 Standalone)

juce_add_plugin(DRMTRGRR
    COMPANY_NAME "yeahwaah"
    BUNDLE_ID "com.yeahwaah.drmtrgrr"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Ywah
    PLUGIN_CODE DTrg
    FORMATS ${PLUGIN_FORMATS}
    PRODUCT_NAME "DRM-TRGRR${PROJECT_VERSION}"
)

target_sources(DRMTRGRR
    PRIVATE
        Source/PluginProcessor.h
        Source/PluginProcessor.cpp
        Source/PluginEditor.h
        Source/PluginEditor.cpp
        Source/DSP/VisualizerBuffer.h
        Source/DSP/TransientDetector.h
        Source/DSP/TransientDetector.cpp
        Source/DSP/DrumSynthesizer.h
        Source/DSP/DrumSynthesizer.cpp
        Source/DSP/TransientGate.h
        Source/DSP/SampleSlot.h
        Source/DSP/SampleSlot.cpp
        Source/UI/SlateLookAndFeel.h
        Source/UI/SlateLookAndFeel.cpp
        Source/UI/HardwareControls.h
        Source/UI/HardwareControls.cpp
        Source/UI/TriggerVisualizer.h
        Source/UI/TriggerVisualizer.cpp
        Source/UI/DetectorPanel.h
        Source/UI/DetectorPanel.cpp
        Source/UI/SlotRackComponent.h
        Source/UI/SlotRackComponent.cpp
        Source/UI/BrowserView.h
        Source/UI/BrowserView.cpp
        Source/UI/SettingsView.h
        Source/UI/SettingsView.cpp
        Source/UI/HeaderBar.h
        Source/UI/HeaderBar.cpp
)

target_compile_definitions(DRMTRGRR
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        NOMINMAX
        _CRT_SECURE_NO_WARNINGS
)

target_link_libraries(DRMTRGRR
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_audio_formats
        juce::juce_audio_basics
        juce::juce_dsp
        juce::juce_gui_extra
        juce::juce_gui_basics
        juce::juce_graphics
        juce::juce_events
        juce::juce_core
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Screenshot Generator Tool
add_executable(generate_screenshot tools/screenshot_generator.cpp)
target_compile_definitions(generate_screenshot PRIVATE JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1)
target_link_libraries(generate_screenshot PRIVATE
    DRMTRGRR
    juce::juce_audio_utils
    juce::juce_gui_basics
    juce::juce_graphics
)
