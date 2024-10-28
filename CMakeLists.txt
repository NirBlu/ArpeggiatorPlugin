cmake_minimum_required(VERSION 3.15)
project(ArpeggiatorPlugin)

# Add DPF framework
add_subdirectory(Libraries/DPF)
include_directories(${CMAKE_SOURCE_DIR}/Source)

# Add source files for the plugin
file(GLOB SRC_FILES Source/*.cpp)

# Define the plugin target
dpf_add_plugin(ArpeggiatorPlugin TARGETS lv2)
target_sources(ArpeggiatorPlugin PRIVATE ${SRC_FILES})

