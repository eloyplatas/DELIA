cmake_minimum_required(VERSION 3.22)
# Antes de project(): si no, no tienen efecto
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "")
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "")
project(Delia VERSION 0.2.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
  add_subdirectory(JUCE)
else()
  include(FetchContent)
  FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.10 GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(JUCE)
endif()

if(APPLE)
  set(DELIA_FORMATS VST3 Standalone)
else()
  set(DELIA_FORMATS VST3 Standalone)
endif()

juce_add_plugin(Delia
  COMPANY_NAME "Eloy Platas"
  PLUGIN_MANUFACTURER_CODE Eplt
  PLUGIN_CODE Deli
  FORMATS ${DELIA_FORMATS}
  PRODUCT_NAME "Delia"
  IS_SYNTH TRUE
  NEEDS_MIDI_INPUT TRUE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS TRUE
  COPY_PLUGIN_AFTER_BUILD FALSE
  VST3_CATEGORIES Instrument Sampler
  AU_MAIN_TYPE kAudioUnitType_MusicDevice)

target_sources(Delia PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_compile_definitions(Delia PUBLIC
  JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0 JUCE_DISPLAY_SPLASH_SCREEN=0
  JUCE_REPORT_APP_USAGE=0 JUCE_USE_FLAC=1 JUCE_USE_OGGVORBIS=1 JUCE_JACK=0)
target_link_libraries(Delia PRIVATE
  juce::juce_audio_utils juce::juce_dsp juce::juce_cryptography
  PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags)

# Prueba sin interfaz: renderiza audio con el motor y comprueba el resultado
option(DELIA_BUILD_TESTS "Compilar la prueba del motor" OFF)
if(DELIA_BUILD_TESTS)
  juce_add_console_app(DeliaTest PRODUCT_NAME "DeliaTest")
  target_sources(DeliaTest PRIVATE Tests/EngineTest.cpp)
  target_compile_definitions(DeliaTest PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_USE_FLAC=1)
  target_link_libraries(DeliaTest PRIVATE juce::juce_audio_formats juce::juce_dsp juce::juce_recommended_config_flags)
endif()

# Herramienta de desarrollo: guarda capturas de la interfaz y comprueba volcado y estado
option(DELIA_BUILD_SHOT "Compilar la herramienta de capturas" OFF)
if(DELIA_BUILD_SHOT)
  add_executable(DeliaShot Tests/Shot.cpp)
  target_include_directories(DeliaShot PRIVATE $<TARGET_PROPERTY:Delia,INCLUDE_DIRECTORIES>)
  target_compile_definitions(DeliaShot PRIVATE $<TARGET_PROPERTY:Delia,COMPILE_DEFINITIONS>)
  target_link_libraries(DeliaShot PRIVATE Delia)
endif()
