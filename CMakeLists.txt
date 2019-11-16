cmake_minimum_required(VERSION 3.11)
SET(CMAKE_CXX_STANDARD 17)

# default layers
#set(ENV{VK_ICD_FILENAMES} "/Users/25granty/Desktop/CLion/Hephaestus/vendor/vulkansdk-macos-1.1.126.0/macOS/etc/vulkan/icd.d/MoltenVK_icd.json")
#set(ENV{VK_LAYER_PATH} "/Users/25granty/Desktop/CLion/Hephaestus/vendor/vulkansdk-macos-1.1.126.0/macOS/etc/vulkan/explicit_layer.d")

set(ENV{VK_ICD_FILENAMES} "/Users/25granty/Desktop/CLion/Mason/Hephaestus/lib/MoltenVK_icd.json")
set(ENV{VK_LAYER_PATH} "/Users/25granty/Desktop/CLion/Mason/Hephaestus/lib/validation_layers.d")

# ======= DEPENDENCY: GLFW =========
#set(GLFW_VULKAN_STATIC ON CACHE BOOL "" FORCE)
#set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
#add_subdirectory(vendor/glfw)

# ======= STATIC LIBRARY BUILDING ========
project(Hephaestus VERSION 1.0.0 LANGUAGES CXX)
add_library(Hephaestus STATIC src/hp/hp.cpp src/hp/hp.h)
target_link_libraries(Hephaestus PUBLIC ${PROJECT_SOURCE_DIR}/lib/libvulkan.1.1.126.dylib)
#target_link_libraries(Hephaestus PUBLIC ${PROJECT_SOURCE_DIR}/lib/libMoltenVK.dylib ${PROJECT_SOURCE_DIR}/lib/libVkLayer_khronos_validation.dylib ${PROJECT_SOURCE_DIR}/lib/libVkLayer_api_dump.dylib)
target_include_directories(Hephaestus PUBLIC include src)
target_link_libraries(Hephaestus PUBLIC ${PROJECT_SOURCE_DIR}/lib/libglfw3.a)

# ====== SHARED LIBRARY BUILDING ========
project(HephaestusShared VERSION 1.0.0 LANGUAGES CXX)
add_library(HephaestusShared SHARED src/hp/hp.cpp src/hp/hp.h)
target_link_libraries(HephaestusShared PUBLIC ${PROJECT_SOURCE_DIR}/lib/libvulkan.1.1.126.dylib)
#target_link_libraries(HephaestusShared PUBLIC ${PROJECT_SOURCE_DIR}/lib/libMoltenVK.dylib ${PROJECT_SOURCE_DIR}/lib/libVkLayer_khronos_validation.dylib ${PROJECT_SOURCE_DIR}/lib/libVkLayer_api_dump.dylib)
target_include_directories(HephaestusShared PUBLIC include src)
target_link_libraries(HephaestusShared PUBLIC ${PROJECT_SOURCE_DIR}/lib/libglfw.3.4.dylib)


# ====== SANDBOX BUILDING ========
add_subdirectory(sandbox)
