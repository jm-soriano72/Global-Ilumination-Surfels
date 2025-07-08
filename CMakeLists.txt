
cmake_minimum_required(VERSION 3.16)

project(VulkanEngine)


# Place inside source dir
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)

# Activate all warnings
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # using Visual Studio C++
    message("Visual Studio C++")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # using GCC
    message("Using GCC")
endif()


# Set the platform (e.g., x64)
set(CMAKE_GENERATOR_PLATFORM x64) # or Win32, as needed

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# For exporting compile config
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) #DOES NOT WORK WITH MSVC

# Find the Vulkan package
find_package(Vulkan REQUIRED)

if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan SDK not found. Please install Vulkan SDK and ensure it's in your CMake search path.")
endif()

# Get the root path for Vulkan from the Vulkan_LIBRARY variable
get_filename_component(VULKAN_SDK_ROOT ${Vulkan_LIBRARY} DIRECTORY)

# Set up dependencies
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(dependencies/glfw)
add_subdirectory(dependencies/glm-1.0.1)
add_subdirectory(dependencies/stb_image)
add_subdirectory(dependencies/assimp)

include_directories(${CMAKE_SOURCE_DIR}/src)	

# Add your own source code
file(GLOB_RECURSE APP_SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.h"
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

# set it up for a exe
add_executable(VulkanEngine  ${APP_SOURCES})

# Configure vulkan path
link_directories(${VULKAN_SDK_ROOT})
# Include directories
target_include_directories(VulkanEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/)
# Link libraries
target_link_libraries(VulkanEngine PUBLIC 

    Vulkan::Vulkan 
    glm 
    glfw 
    stb_image 
    assimp
)

# Set dependencies inside folder
set_property(TARGET glfw glm stb_image PROPERTY FOLDER "thirdparty")

# Set resources path
target_compile_definitions(VulkanEngine PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/")

