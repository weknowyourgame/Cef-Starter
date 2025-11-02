cmake_minimum_required(VERSION 3.18)

# Set default build type BEFORE project() for single-config generators
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Project configuration - can be overridden via command line
# Example: cmake -DPROJECT_NAME=my_app -DEXECUTABLE_NAME=my_app ..
if(NOT DEFINED PROJECT_NAME)
  set(PROJECT_NAME "cef_starter")
endif()

if(NOT DEFINED EXECUTABLE_NAME)
  set(EXECUTABLE_NAME "${PROJECT_NAME}")
endif()

project(${PROJECT_NAME} C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Application configuration options
option(USE_SANDBOX "Enable CEF sandbox" OFF)
option(CEF_ENABLE_OFFSCREEN "Enable off-screen rendering support" OFF)

# Default URL and window title - can be overridden via command line
# Example: cmake -DDEFAULT_URL=https://example.com -DWINDOW_TITLE="My App" ..
if(NOT DEFINED DEFAULT_URL)
  set(DEFAULT_URL "https://www.chromium.org")
endif()

if(NOT DEFINED WINDOW_TITLE)
  set(WINDOW_TITLE "${PROJECT_NAME}")
endif()

# CEF_ROOT can be set via -DCEF_ROOT=/path/to/cef_binary_XXX
# Or auto-detect from common locations
if(NOT DEFINED CEF_ROOT)
  set(CEF_SEARCH_PATHS
    "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/cef"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/cef"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/cef"
    "$ENV{CEF_ROOT}"
  )
  
  foreach(SEARCH_PATH ${CEF_SEARCH_PATHS})
    if(EXISTS "${SEARCH_PATH}")
      file(GLOB VERSIONED_DIRS "${SEARCH_PATH}/cef_binary_*")
      if(VERSIONED_DIRS)
        list(GET VERSIONED_DIRS 0 CEF_ROOT)
        break()
      endif()
    endif()
  endforeach()
endif()

if(NOT DEFINED CEF_ROOT OR NOT EXISTS "${CEF_ROOT}")
  message(FATAL_ERROR "CEF not found. Set -DCEF_ROOT=/path/to/cef_binary_XXX")
endif()

# Include FindCEF to set up all CEF variables (wrapper needs these)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CEF_ROOT}/cmake")
include(FindCEF)

# If FindCEF didn't set variables, set them manually
# CEF_INCLUDE_DIR should point to CEF_ROOT, not CEF_ROOT/include
# because headers are included as "include/cef_app.h"
if(NOT DEFINED CEF_INCLUDE_DIR)
  set(CEF_INCLUDE_DIR "${CEF_ROOT}")
endif()

if(NOT DEFINED CEF_LIBRARY_DIR)
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CEF_LIBRARY_DIR "${CEF_ROOT}/Debug")
  else()
    set(CEF_LIBRARY_DIR "${CEF_ROOT}/Release")
  endif()
endif()

if(NOT DEFINED CEF_LIBRARY)
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CEF_LIBRARY "${CEF_LIBRARY_DIR}/libcef.so")
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(CEF_LIBRARY "${CEF_LIBRARY_DIR}/libcef.dll")
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(CEF_LIBRARY "${CEF_ROOT}/Release/Chromium Embedded Framework.framework")
  endif()
endif()

if(NOT DEFINED CEF_LIBCEF_DLL_WRAPPER_PATH)
  set(CEF_LIBCEF_DLL_WRAPPER_PATH "${CEF_ROOT}/libcef_dll")
endif()

if(NOT DEFINED CEF_RESOURCE_DIR)
  set(CEF_RESOURCE_DIR "${CEF_ROOT}/Resources")
endif()

if(NOT DEFINED CEF_BINARY_DIR)
  set(CEF_BINARY_DIR "${CEF_LIBRARY_DIR}")
endif()

# Verify paths exist
if(NOT EXISTS "${CEF_INCLUDE_DIR}")
  message(FATAL_ERROR "CEF include directory not found: ${CEF_INCLUDE_DIR}")
endif()

if(NOT EXISTS "${CEF_LIBRARY}")
  message(FATAL_ERROR "CEF library not found: ${CEF_LIBRARY}")
endif()

if(NOT EXISTS "${CEF_LIBCEF_DLL_WRAPPER_PATH}/CMakeLists.txt")
  message(FATAL_ERROR "CEF wrapper not found: ${CEF_LIBCEF_DLL_WRAPPER_PATH}")
endif()

message(STATUS "=== CEF Starter Configuration ===")
message(STATUS "Project Name: ${PROJECT_NAME}")
message(STATUS "Executable Name: ${EXECUTABLE_NAME}")
message(STATUS "Default URL: ${DEFAULT_URL}")
message(STATUS "Window Title: ${WINDOW_TITLE}")
message(STATUS "Use Sandbox: ${USE_SANDBOX}")
message(STATUS "CEF_ROOT: ${CEF_ROOT}")
message(STATUS "================================")

# Source files
set(SOURCES
  app.cpp
  app.h
  client.cpp
  client.h
  main.cpp
)

# Executable
add_executable(${EXECUTABLE_NAME} ${SOURCES})

target_include_directories(${EXECUTABLE_NAME} PRIVATE ${CEF_INCLUDE_DIR})

# Define compile-time configuration
target_compile_definitions(${EXECUTABLE_NAME} PRIVATE
  DEFAULT_URL="${DEFAULT_URL}"
  WINDOW_TITLE="${WINDOW_TITLE}"
)

if(USE_SANDBOX)
  target_compile_definitions(${EXECUTABLE_NAME} PRIVATE CEF_USE_SANDBOX=1)
endif()

if(CEF_ENABLE_OFFSCREEN)
  target_compile_definitions(${EXECUTABLE_NAME} PRIVATE CEF_ENABLE_OFFSCREEN=1)
endif()

# Build libcef_dll_wrapper
add_subdirectory(${CEF_LIBCEF_DLL_WRAPPER_PATH} libcef_dll_wrapper)

# Link libraries
target_link_libraries(${EXECUTABLE_NAME}
  libcef_dll_wrapper
  ${CEF_LIBRARY}
)

# Platform-specific configuration
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(${EXECUTABLE_NAME} X11)
  
  set_target_properties(${EXECUTABLE_NAME} PROPERTIES
    INSTALL_RPATH "$ORIGIN"
    BUILD_WITH_INSTALL_RPATH TRUE
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
  )
  
  # Copy CEF files
  add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${CEF_BINARY_DIR}/libcef.so" "${CMAKE_BINARY_DIR}/libcef.so"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${CEF_BINARY_DIR}/chrome-sandbox" "${CMAKE_BINARY_DIR}/chrome-sandbox"
    COMMAND chmod u+s "${CMAKE_BINARY_DIR}/chrome-sandbox"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${CEF_BINARY_DIR}/v8_context_snapshot.bin" "${CMAKE_BINARY_DIR}/v8_context_snapshot.bin"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${CEF_RESOURCE_DIR}" "${CMAKE_BINARY_DIR}"
  )
  
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set_target_properties(${EXECUTABLE_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
  )
  
  add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${CEF_BINARY_DIR}/${CMAKE_BUILD_TYPE}/libcef.dll" "${CMAKE_BINARY_DIR}/libcef.dll"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${CEF_RESOURCE_DIR}" "${CMAKE_BINARY_DIR}"
  )
  
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  find_library(CEF_FRAMEWORK "Chromium Embedded Framework"
    PATHS "${CEF_BINARY_DIR}" NO_DEFAULT_PATH)
  
  target_link_libraries(${EXECUTABLE_NAME}
    ${CEF_FRAMEWORK}
    "-framework Cocoa"
    "-framework Security"
    "-framework SystemConfiguration"
  )
  
  set_target_properties(${EXECUTABLE_NAME} PROPERTIES
    MACOSX_BUNDLE TRUE
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
  )
  
  add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
      "${CMAKE_BINARY_DIR}/${EXECUTABLE_NAME}.app/Contents/Frameworks"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${CEF_BINARY_DIR}/Chromium Embedded Framework.framework"
      "${CMAKE_BINARY_DIR}/${EXECUTABLE_NAME}.app/Contents/Frameworks/Chromium Embedded Framework.framework"
  )
endif()
