cmake_minimum_required(VERSION 3.11)
SET(CMAKE_CXX_STANDARD 17)

project(HephaestusSandbox VERSION 1.0.0 LANGUAGES CXX)

#find_library(COCOA_LIBRARY Cocoa)

set(ENV{VK_ICD_FILENAMES} "/Users/25granty/Desktop/CLion/Mason/Hephaestus/lib/MoltenVK_icd.json")
set(ENV{VK_LAYER_PATH} "/Users/25granty/Desktop/CLion/Mason/Hephaestus/lib/explicit_layer.d")

add_executable(HephaestusSandbox src/main.cpp)
#target_link_libraries(HephaestusSandbox ${COCOA_LIBRARY})

target_link_libraries(HephaestusSandbox ${PROJECT_SOURCE_DIR}/../cmake-build-debug/libHephaestusShared.dylib ${PROJECT_SOURCE_DIR}/../lib/libvulkan.1.1.126.dylib glfw spdlog)
target_link_directories(HephaestusSandbox PUBLIC ${PROJECT_SOURCE_DIR}/../cmake-build-debug)
#target_link_libraries(HephaestusSandbox ${PROJECT_SOURCE_DIR}/../lib/libMoltenVK.dylib
#        ${PROJECT_SOURCE_DIR}/../lib/libVkLayer_khronos_validation.dylib ${PROJECT_SOURCE_DIR}/../lib/libVkLayer_api_dump.dylib)

target_include_directories(HephaestusSandbox PUBLIC ../include ../src)