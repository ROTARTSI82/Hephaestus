cmake_minimum_required(VERSION 3.11)
SET(CMAKE_CXX_STANDARD 17)

set(ENV{VK_ICD_FILENAMES} "/Users/25granty/Desktop/CLion/Mason/Hephaestus/lib/MoltenVK_icd.json")
set(ENV{VK_LAYER_PATH} "/Users/25granty/Desktop/CLion/Mason/Hephaestus/lib/explicit_layer.d")

# ======= DEPENDENCY: GLFW =========
set(GLFW_VULKAN_STATIC ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
add_subdirectory(vendor/glfw)

# ======= DEPENDENCY: SPDLOG ========
add_subdirectory(vendor/spdlog)

# ======= STATIC LIBRARY BUILDING ========  DO NOT USE THIS IS BROKEN! This compile is skipped for now...
#project(BROKEN_DO_NOT_USE_HephaestusStatic VERSION 1.0.0 LANGUAGES CXX)
#add_library(BROKEN_DO_NOT_USE_HephaestusStatic STATIC api/hp/hp.hpp src/hp/profiling.cpp api/hp/profiling.hpp api/hp/config.hpp src/hp/logging.cpp api/hp/logging.hpp src/hp/vk/instance.cpp api/hp/vk/instance.hpp src/hp/vk/vk.cpp api/hp/vk/vk.hpp)
#target_link_libraries(BROKEN_DO_NOT_USE_HephaestusStatic PUBLIC ${PROJECT_SOURCE_DIR}/lib/vulkan/macOS/lib/libvulkan.1.1.126.dylib spdlog)
#target_include_directories(BROKEN_DO_NOT_USE_HephaestusStatic PUBLIC include src api)
#target_link_libraries(BROKEN_DO_NOT_USE_HephaestusStatic PUBLIC glfw)

# ====== SHARED LIBRARY BUILDING ========
project(HephaestusShared VERSION 1.0.0 LANGUAGES CXX)
add_library(HephaestusShared SHARED api/hp/hp.hpp src/hp/profiling.cpp api/hp/profiling.hpp api/hp/config.hpp src/hp/logging.cpp api/hp/logging.hpp src/hp/vk/instance.cpp api/hp/vk/instance.hpp src/hp/vk/vk.cpp api/hp/vk/vk.hpp)
target_link_libraries(HephaestusShared PUBLIC ${PROJECT_SOURCE_DIR}/lib/vulkan/macOS/lib/libvulkan.1.1.126.dylib spdlog)
target_include_directories(HephaestusShared PUBLIC include src api)
target_link_libraries(HephaestusShared PUBLIC glfw)


# ====== SANDBOX BUILDING ========
add_subdirectory(sandbox)
