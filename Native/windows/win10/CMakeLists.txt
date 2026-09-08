cmake_minimum_required(VERSION 3.20)

project(CamStudioWin10
    VERSION 1.0
    LANGUAGES CXX
)

# ============================================================
# Output directories
# ============================================================

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    "${CMAKE_CURRENT_SOURCE_DIR}/dist"
)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
    "${CMAKE_CURRENT_SOURCE_DIR}/dist"
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
    "${CMAKE_CURRENT_SOURCE_DIR}/dist"
)

foreach(CONFIG Debug Release)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG}
        "${CMAKE_CURRENT_SOURCE_DIR}/dist/${CONFIG}"
    )

    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG}
        "${CMAKE_CURRENT_SOURCE_DIR}/dist/${CONFIG}"
    )

    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG}
        "${CMAKE_CURRENT_SOURCE_DIR}/dist/${CONFIG}"
    )
endforeach()


# ============================================================
# Architecture
# ============================================================

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR
        "CamStudio Win10 requires a 64-bit build."
    )
endif()


# ============================================================
# DirectShow BaseClasses
# ============================================================

set(DIRECTSHOW_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/directshow/baseclasses"
)

set(DIRECTSHOW_INCLUDE_DIR
    "${DIRECTSHOW_DIR}"
)

if(NOT EXISTS "${DIRECTSHOW_INCLUDE_DIR}/streams.h")
    message(FATAL_ERROR
        "DirectShow BaseClasses not found: ${DIRECTSHOW_INCLUDE_DIR}"
    )
endif()


# ============================================================
# Sources
# ============================================================

set(CAMSTUDIO_SOURCES

    src/DllMain.cpp

    src/CamStudioNative.cpp

    src/CamStudioSharedMemory.cpp

    src/Windows10VirtualCamera.cpp
)


set(CAMSTUDIO_HEADERS

    src/framework.h

    src/CamStudioNative.h
    src/CamStudioSharedMemory.h
    src/Windows10VirtualCamera.h
)


# ============================================================
# Virtual Camera DLL
# ============================================================

add_library(
    CamStudioVirtualCamera
    SHARED

    ${CAMSTUDIO_SOURCES}
    ${CAMSTUDIO_HEADERS}

    CamStudioVirtualCamera.def
)


# ============================================================
# Include directories
# ============================================================

target_include_directories(
    CamStudioVirtualCamera
    PRIVATE

    ${DIRECTSHOW_INCLUDE_DIR}

    ${CMAKE_CURRENT_SOURCE_DIR}/src
)


# ============================================================
# Preprocessor definitions
# ============================================================

target_compile_definitions(
    CamStudioVirtualCamera
    PRIVATE

    WIN32

    UNICODE
    _UNICODE

    CAMSTUDIOVIRTUALCAMERA_EXPORTS

    _WINDOWS
    _USRDLL
)


# ============================================================
# C++ configuration
# ============================================================

set_target_properties(
    CamStudioVirtualCamera
    PROPERTIES

    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON

    CXX_EXTENSIONS OFF

    OUTPUT_NAME
        "CamStudioVirtualCamera"

    PREFIX ""
)


# ============================================================
# Compiler configuration
# ============================================================

if(MSVC)

    target_compile_options(
        CamStudioVirtualCamera
        PRIVATE

        /W3
        /EHsc
        /MP
    )

endif()


# ============================================================
# DirectShow libraries
#
# $<CONFIG> automatically selects:
#
# x64/Debug
# x64/Release
# ============================================================

target_link_directories(
    CamStudioVirtualCamera
    PRIVATE

    "${DIRECTSHOW_DIR}/x64/$<CONFIG>"
)


# ============================================================
# Link libraries
# ============================================================

set(DIRECTSHOW_BASE_LIB
    "${DIRECTSHOW_DIR}/x64/$<IF:$<CONFIG:Debug>,Debug/strmbasd.lib,Release/strmbase.lib>"
)

target_link_libraries(
    CamStudioVirtualCamera
    PRIVATE

    "${DIRECTSHOW_BASE_LIB}"
    strmiids
    winmm

    kernel32
    user32
    gdi32
    winspool
    shell32
    ole32
    oleaut32
    uuid
    comdlg32
    advapi32
)


# ============================================================
# Linker configuration
# ============================================================

if(MSVC)

    target_link_options(
        CamStudioVirtualCamera
        PRIVATE

        /INCREMENTAL:NO
    )

endif()


# ============================================================
# Module definition file
# ============================================================

set_target_properties(
    CamStudioVirtualCamera
    PROPERTIES

    LINK_FLAGS
        "/DEF:\"${CMAKE_CURRENT_SOURCE_DIR}/CamStudioVirtualCamera.def\""
)


# ============================================================
# Configuration messages
# ============================================================

message(STATUS "")
message(STATUS "==============================================")
message(STATUS " Cam Studio - Windows 10 Virtual Camera")
message(STATUS "==============================================")
message(STATUS "Architecture: x64")
message(STATUS "DirectShow:   ${DIRECTSHOW_DIR}")
message(STATUS "Output:       ${CMAKE_CURRENT_SOURCE_DIR}/dist")
message(STATUS "==============================================")
message(STATUS "")