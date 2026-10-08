cmake_minimum_required(VERSION 3.22)

project(LiveStreamMicroDAW VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/utf-8)
endif()

# ==============================================================================
# JUCE 8 Dependency Configuration
# ==============================================================================
# Fetch JUCE 8 via CPM or FetchContent if not provided locally
include(FetchContent)

set(JUCE_ENABLE_MODULE_SOURCE_GROUPS ON CACHE BOOL "" FORCE)
set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4 # Target JUCE 8 stable release
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

# ==============================================================================
# Steinberg ASIO SDK Configuration
# ==============================================================================
# Instructions for Steinberg ASIO SDK:
# Auto-detect bundled third_party/asiosdk or user provided path
if(NOT DEFINED ASIO_SDK_DIR OR ASIO_SDK_DIR STREQUAL "")
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/asiosdk/common/asiosys.h")
        set(ASIO_SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/asiosdk" CACHE PATH "Path to Steinberg ASIO SDK directory" FORCE)
    endif()
endif()

set(JUCE_ASIO_AVAILABLE OFF)
if(WIN32)
    if(EXISTS "${ASIO_SDK_DIR}/common/asiosys.h")
        message(STATUS "Found Steinberg ASIO SDK at: ${ASIO_SDK_DIR}")
        set(JUCE_ASIO_AVAILABLE ON)
    elseif(DEFINED ENV{ASIO_SDK_DIR} AND EXISTS "$ENV{ASIO_SDK_DIR}/common/asiosys.h")
        set(ASIO_SDK_DIR "$ENV{ASIO_SDK_DIR}" CACHE PATH "Path to Steinberg ASIO SDK directory" FORCE)
        message(STATUS "Found Steinberg ASIO SDK from environment at: ${ASIO_SDK_DIR}")
        set(JUCE_ASIO_AVAILABLE ON)
    else()
        message(WARNING "Steinberg ASIO SDK not found! Set -DASIO_SDK_DIR=\"path/to/asiosdk\" to compile with ASIO support. Falling back to WASAPI/DirectSound only.")
    endif()
endif()

# ==============================================================================
# Shared IPC Library (Lock-Free Ring Buffer & Shared Memory Definition)
# ==============================================================================
add_library(LiveStreamIPC INTERFACE)
target_include_directories(LiveStreamIPC INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ipc
)
target_compile_features(LiveStreamIPC INTERFACE cxx_std_20)

juce_add_binary_data(AppBinaryData
    SOURCES
    assets/icon.png
    assets/qr_vietqr.png
    assets/qr_momo.jpg
    assets/qr_mbbank.jpg
    assets/qr_paypal.jpg
    assets/songbook.json
)

# ==============================================================================
# Common DAW Source Files
# ==============================================================================
set(COMMON_DAW_SOURCES
    src/main.cpp
    src/MainComponent.cpp
    src/MainComponent.h
    src/audio/AudioEngine.cpp
    src/audio/AudioEngine.h
    src/audio/GraphManager.cpp
    src/audio/GraphManager.h
    src/audio/KeyDetector.cpp
    src/audio/KeyDetector.h
    src/audio/BeatPlayerAudioProcessor.cpp
    src/audio/BeatPlayerAudioProcessor.h
    src/audio/IpcEmitterAudioProcessor.cpp
    src/audio/IpcEmitterAudioProcessor.h
    src/ipc/SharedMemoryAudioBuffer.h
    src/presets/PresetManager.cpp
    src/presets/PresetManager.h
    src/songbook/SongbookManager.cpp
    src/songbook/SongbookManager.h
    src/ui/VerticalRackComponent.cpp
    src/ui/VerticalRackComponent.h
    src/ui/PluginSlotComponent.cpp
    src/ui/PluginSlotComponent.h
    src/ui/MasterChannelStripComponent.cpp
    src/ui/MasterChannelStripComponent.h
    src/ui/AudioSettingsOverlay.cpp
    src/ui/AudioSettingsOverlay.h
    src/ui/DonateOverlay.cpp
    src/ui/DonateOverlay.h
    src/ui/SongbookOverlay.cpp
    src/ui/SongbookOverlay.h
    src/ui/KeyDetectorComponent.cpp
    src/ui/KeyDetectorComponent.h
    src/audio/SoundboardAudioProcessor.cpp
    src/audio/SoundboardAudioProcessor.h
    src/ui/SoundboardComponent.cpp
    src/ui/SoundboardComponent.h
    src/audio/AiNoiseSuppressor.cpp
    src/audio/AiNoiseSuppressor.h
    src/audio/AiVocalProfiler.cpp
    src/audio/AiVocalProfiler.h
    src/ui/AiVocalProfilerOverlay.cpp
    src/ui/AiVocalProfilerOverlay.h
    src/audio/VocalRangeDetector.cpp
    src/audio/VocalRangeDetector.h
    src/ui/AiVocalRangeOverlay.cpp
    src/ui/AiVocalRangeOverlay.h
    src/audio/BuiltInDspAudioProcessor.cpp
    src/audio/BuiltInDspAudioProcessor.h

    src/ui/BuiltInDspComponent.cpp
    src/ui/BuiltInDspComponent.h
    src/audio/TempoSyncEngine.cpp
    src/audio/TempoSyncEngine.h
    src/audio/AudioRecorder.cpp
    src/audio/AudioRecorder.h
)

# ==============================================================================
# Target 1: Standalone Micro-DAW FREE (Community Edition v2.x)
# ==============================================================================
juce_add_gui_app(LiveStreamMicroDAW
    PRODUCT_NAME "LiveStream Micro-DAW"
    COMPANY_NAME "LiveStreamAudio"
    BUNDLE_IDENTIFIER "com.livestreamaudio.microdaw"
    ICON_BIG "assets/icon.png"
    ICON_SMALL "assets/icon.png"
)

target_sources(LiveStreamMicroDAW PRIVATE ${COMMON_DAW_SOURCES})

target_compile_definitions(LiveStreamMicroDAW PRIVATE
    HOSI_PRO_EDITION=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_PLUGINHOST_VST3=1
)

if(WIN32)
    target_compile_definitions(LiveStreamMicroDAW PRIVATE
        JUCE_PLUGINHOST_AU=0
        JUCE_WASAPI=1
        JUCE_DIRECTSOUND=1
    )
elseif(APPLE)
    target_compile_definitions(LiveStreamMicroDAW PRIVATE
        JUCE_PLUGINHOST_AU=1
        JUCE_COREAUDIO=1
    )
endif()

if(JUCE_ASIO_AVAILABLE)
    target_compile_definitions(LiveStreamMicroDAW PRIVATE JUCE_ASIO=1)
    target_include_directories(LiveStreamMicroDAW PRIVATE 
        ${ASIO_SDK_DIR}/common
        ${ASIO_SDK_DIR}/host
        ${ASIO_SDK_DIR}/host/pc
    )
else()
    target_compile_definitions(LiveStreamMicroDAW PRIVATE JUCE_ASIO=0)
endif()

target_link_libraries(LiveStreamMicroDAW PRIVATE
    LiveStreamIPC
    AppBinaryData
    juce::juce_core
    juce::juce_events
    juce::juce_graphics
    juce::juce_data_structures
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_dsp
)

# ==============================================================================
# Target 2: Standalone Micro-DAW PRO (Commercial Studio AI Edition v3.x)
# ==============================================================================
juce_add_gui_app(LiveStreamMicroDAW_PRO
    PRODUCT_NAME "LiveStream Micro-DAW PRO"
    COMPANY_NAME "LiveStreamAudio"
    BUNDLE_IDENTIFIER "com.livestreamaudio.microdaw.pro"
    ICON_BIG "assets/icon.png"
    ICON_SMALL "assets/icon.png"
)

target_sources(LiveStreamMicroDAW_PRO PRIVATE
    ${COMMON_DAW_SOURCES}
    src/ui/YouTubePlayerOverlay.cpp
    src/ui/YouTubePlayerOverlay.h
)

target_compile_definitions(LiveStreamMicroDAW_PRO PRIVATE
    HOSI_PRO_EDITION=1
    JUCE_WEB_BROWSER=1
    JUCE_USE_CURL=0
    JUCE_PLUGINHOST_VST3=1
)

if(WIN32)
    target_compile_definitions(LiveStreamMicroDAW_PRO PRIVATE
        JUCE_PLUGINHOST_AU=0
        JUCE_WASAPI=1
        JUCE_DIRECTSOUND=1
        JUCE_USE_WIN_WEBVIEW2=1
        JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING=1
    )
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/webview2/build/native/include/WebView2.h")
        target_include_directories(LiveStreamMicroDAW_PRO PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/third_party/webview2/build/native/include
        )
        target_link_libraries(LiveStreamMicroDAW_PRO PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/third_party/webview2/build/native/x64/WebView2LoaderStatic.lib"
            version
        )
    endif()
elseif(APPLE)
    target_compile_definitions(LiveStreamMicroDAW_PRO PRIVATE
        JUCE_PLUGINHOST_AU=1
        JUCE_COREAUDIO=1
    )
endif()

if(JUCE_ASIO_AVAILABLE)
    target_compile_definitions(LiveStreamMicroDAW_PRO PRIVATE JUCE_ASIO=1)
    target_include_directories(LiveStreamMicroDAW_PRO PRIVATE 
        ${ASIO_SDK_DIR}/common
        ${ASIO_SDK_DIR}/host
        ${ASIO_SDK_DIR}/host/pc
    )
else()
    target_compile_definitions(LiveStreamMicroDAW_PRO PRIVATE JUCE_ASIO=0)
endif()

target_link_libraries(LiveStreamMicroDAW_PRO PRIVATE
    LiveStreamIPC
    AppBinaryData
    juce::juce_core
    juce::juce_events
    juce::juce_graphics
    juce::juce_data_structures
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_dsp
)


# ==============================================================================
# Target 2: OBS Receiver Plugin (VST2 for OBS Studio & VST3/AU for DAWs)
# ==============================================================================
if(APPLE)
    set(RECEIVER_FORMATS VST3 AU)
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/vst2sdk/pluginterfaces/vst2.x/aeffect.h")
    juce_set_vst2_sdk_path("${CMAKE_CURRENT_SOURCE_DIR}/third_party/vst2sdk")
    set(RECEIVER_FORMATS VST VST3)
else()
    set(RECEIVER_FORMATS VST3)
endif()

juce_add_plugin(LiveStreamOBSReceiver
    COMPANY_NAME "LiveStreamAudio"
    PLUGIN_MANUFACTURER_CODE LsAu
    PLUGIN_CODE LsRc
    FORMATS ${RECEIVER_FORMATS}
    PRODUCT_NAME "LiveStream OBS Receiver"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

target_sources(LiveStreamOBSReceiver PRIVATE
    plugins/receiver/PluginProcessor.cpp
    plugins/receiver/PluginProcessor.h
    plugins/receiver/PluginEditor.cpp
    plugins/receiver/PluginEditor.h
    src/ipc/SharedMemoryAudioBuffer.h
)

target_compile_definitions(LiveStreamOBSReceiver PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(LiveStreamOBSReceiver PRIVATE
    LiveStreamIPC
    juce::juce_core
    juce::juce_events
    juce::juce_graphics
    juce::juce_data_structures
    juce::juce_gui_basics
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_audio_utils
)
