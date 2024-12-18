cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME boost_converter)

# Include toolchain file
include("cmake/gcc-arm-none-eabi.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Enable CMake support for ASM, C++ and C languages
enable_language(C CXX ASM)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Create an executable object type
file(GLOB_RECURSE SRC_C_FILES "src/*.c")
file(GLOB_RECURSE LIB_C_FILES "../libs/*.c")
add_executable(${CMAKE_PROJECT_NAME} ${SRC_C_FILES} ${LIB_C_FILES})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
)

# Function to recursively add all subdirectories to the include path
function(add_include_subdirectories root_dir)
  file(GLOB children RELATIVE ${root_dir} ${root_dir}/*)
  foreach(child ${children})
    if(IS_DIRECTORY ${root_dir}/${child})
      message(STATUS "Adding directory ${root_dir}/${child} to include directories")
      include_directories(${root_dir}/${child})
      add_include_subdirectories(${root_dir}/${child})
    endif()
  endforeach()
endfunction()

# Add include paths
add_include_subdirectories("${CMAKE_CURRENT_SOURCE_DIR}/../libs/core")
add_include_subdirectories("${CMAKE_CURRENT_SOURCE_DIR}/../libs/device")
add_include_subdirectories("${CMAKE_CURRENT_SOURCE_DIR}/src")

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    #libs/
    src/
    # DSP added
    Drivers/CMSIS/DSP/Include
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
    # Add user defined libraries
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/DSP/Lib/GCC/libarm_cortexM4lf_math.a
)
