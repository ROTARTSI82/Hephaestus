cmake_minimum_required(VERSION 3.11)
SET(CMAKE_CXX_STANDARD 17)

project(HephaestusSandbox VERSION 1.0.0 LANGUAGES CXX)

#set(ENV{VK_ICD_FILENAMES} "/Users/25granty/Desktop/CLion/Mason/Hephaestus/lib/MoltenVK_icd.json")
#set(ENV{VK_LAYER_PATH} "/Users/25granty/Desktop/CLion/Mason/Hephaestus/lib/explicit_layer.d")

add_executable(HephaestusSandbox main.cpp)

# exp code

# mac specific code. Comment out if on windows.
#target_link_libraries(HephaestusSandbox ${PROJECT_SOURCE_DIR}/../cmake-build-debug/libHephaestusShared.dylib)
#target_link_libraries(HephaestusSandbox ${PROJECT_SOURCE_DIR}/../vendor/lunarg-vulkan-sdk/macOS/lib/libvulkan.1.1.126.dylib)
#target_include_directories(HephaestusSandbox PUBLIC ../vendor/lunarg-vulkan-sdk/macOS/include)

#windows specific code. comment out if on mac.
target_link_directories(HephaestusSandbox PRIVATE C:\\Users\\grant\\Desktop\\VisualStudio\\Hephaestus\\cmake-build-debug)
target_link_libraries(HephaestusSandbox C:\\Users\\grant\\Desktop\\VisualStudio\\Hephaestus\\cmake-build-debug\\HephaestusStatic.lib)  # Modify to point to lib
target_link_libraries(HephaestusSandbox $ENV{VULKAN_SDK}\\Lib32\\vulkan-1.lib)
target_link_directories(HephaestusSandbox PRIVATE $ENV{VULKAN_SDK}\\Lib32)
target_include_directories(HephaestusSandbox PRIVATE $ENV{VULKAN_SDK}/Include)

# Platform agnostic code.
target_link_libraries(HephaestusSandbox spdlog glfw glm ${Boost_LIBRARIES})
target_include_directories(HephaestusSandbox PRIVATE ../src ../include ../vendor/glfw/include ../vendor/glm ../vendor/spdlog/include ../vendor ${Boost_INCLUDE_DIR} ../vendor/vma/src)
