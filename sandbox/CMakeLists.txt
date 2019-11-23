cmake_minimum_required(VERSION 3.11)
SET(CMAKE_CXX_STANDARD 17)

project(HephaestusSandbox VERSION 1.0.0 LANGUAGES CXX)

set(ENV{VK_ICD_FILENAMES} "/Users/25granty/Desktop/CLion/Mason/Hephaestus/lib/MoltenVK_icd.json")
set(ENV{VK_LAYER_PATH} "/Users/25granty/Desktop/CLion/Mason/Hephaestus/lib/explicit_layer.d")

add_executable(HephaestusSandbox src/main.cpp)

target_link_libraries(HephaestusSandbox ${PROJECT_SOURCE_DIR}/../cmake-build-debug/libHephaestusShared.dylib)
target_link_libraries(HephaestusSandbox ${PROJECT_SOURCE_DIR}/../vendor/lunarg-vulkan-sdk/macOS/lib/libvulkan.1.1.126.dylib glfw spdlog)
target_link_directories(HephaestusSandbox PUBLIC ${PROJECT_SOURCE_DIR}/../cmake-build-debug)

target_include_directories(HephaestusSandbox PUBLIC ../src ../api ../vendor/glfw/include ../vendor/glm ../vendor/spdlog/include ../vendor ../vendor/lunarg-vulkan-sdk/macOS/include ../vendor/boost)