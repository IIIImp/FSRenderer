cmake_minimum_required(VERSION 3.20)
project(FSRenderer)

set(THIRDPARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Advanced CMake Files
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Find Vulkan SDK
message(STATUS "VULKAN_SDK: $ENV{VULKAN_SDK}")
set(Vulkan_INCLUDE_DIRS "$ENV{VULKAN_SDK}/Include")
set(Vulkan_LIBRARIES "$ENV{VULKAN_SDK}/Lib/vulkan-1.lib")
if(NOT TARGET Vulkan::Vulkan)
    add_library(Vulkan::Vulkan UNKNOWN IMPORTED)
    set_target_properties(Vulkan::Vulkan PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${Vulkan_INCLUDE_DIRS}"
        IMPORTED_LOCATION "${Vulkan_LIBRARIES}"
    )
    message(STATUS "Manually created Vulkan::Vulkan target")
    message(STATUS "Vulkan found, version: ${Vulkan_VERSION}")
    message(STATUS "Vulkan include directories: ${Vulkan_INCLUDE_DIRS}")
    message(STATUS "Vulkan libraries: ${Vulkan_LIBRARIES}")
endif()

# ThirParty Support
# GLFW
set(GLFW_INCLUDE_DIRS "${THIRDPARTY_DIR}/GLFW/include")
set(GLFW_LIBRARIES "${THIRDPARTY_DIR}/GLFW/lib-vc2022")
add_library(glfw STATIC IMPORTED)
set_target_properties(glfw PROPERTIES
    IMPORTED_LOCATION "${THIRDPARTY_DIR}/GLFW/lib-vc2022/glfw3.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${GLFW_INCLUDE_DIRS}"
)

# GLM
set(GLM_INCLUDE_DIRS "${THIRDPARTY_DIR}/GLM")

# VMA
set(VMA_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/VMA/include")

# Shader Compiler
# include(CompileShaders)

# ======== Core Lib ============
set(CORE_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${Vulkan_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIRS}
    ${VMA_INCLUDE_DIR}
)

file(GLOB_RECURSE CORE_SOURCES
    "Src/Core/*.cpp"
    "sSrc/Core/*.c"
    "Src/Core/*.h"
    "Src/Core/*.hpp"
)
add_library(CoreLib STATIC ${CORE_SOURCES})


# ================== SAMPLE PART ==========
function(add_sample_project project_name)
    file(GLOB_RECURSE PROJECT_SOURCES
        "src/Samples/${project_name}/*.cpp"
        "src/Samples/${project_name}/*.c"
        "src/Samples/${project_name}/*.h"
        "src/Samples/${project_name}/*.hpp"
    )

    add_executable(${project_name} ${PROJECT_SOURCES})
    
    set_target_properties(${project_name} PROPERTIES
        OUTPUT_NAME ${project_name}
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/samples/${project_name}
    )
    
    target_include_directories(${project_name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Samples/${project_name}
        ${CORE_INCLUDES}
    )
    
    target_link_libraries(${project_name} PRIVATE
        CoreLib
        glfw
        Vulkan::Vulkan
    )
    
    add_custom_command(TARGET ${project_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            $<TARGET_FILE_DIR:${project_name}>/Assets/${project_name}
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/Assets/${project_name}"
            $<TARGET_FILE_DIR:${project_name}>/Assets/${project_name}
    )

    set_property(TARGET ${project_name} PROPERTY FOLDER "Samples")
    set_property(TARGET ${project_name} PROPERTY VS_PROJECT_DESCRIPTION "Vulkan Sample: ${project_name}")
endfunction()

if(TARGET ZERO_CHECK)
    set_target_properties(ZERO_CHECK PROPERTIES FOLDER "CMake")
endif()

if(TARGET ALL_BUILD)
    set_target_properties(ALL_BUILD PROPERTIES FOLDER "CMake")
endif()
set_target_properties(CoreLib PROPERTIES FOLDER "Core")

file(GLOB SAMPLE_PROJECTS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/src/Samples" 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Samples/*")

foreach(PROJECT_PATH ${SAMPLE_PROJECTS})
    if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/Samples/${PROJECT_PATH}")
        message(STATUS "Add Project: ${PROJECT_PATH}")
        add_sample_project(${PROJECT_PATH})
    endif()
    set_target_properties(${SAMPLE} PROPERTIES FOLDER "Samples")
endforeach()