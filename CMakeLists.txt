cmake_minimum_required(VERSION 3.12)
project(HoloLithPlana)

# ===============================================
# Global settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)



# ===============================================
# Source files
file(GLOB_RECURSE SOURCES
    "src/*.cc"
    "src/*.hh"
    "src/*.hpp"
    "src/*.cpp"
    "src/*.h"
    "src/*.fsh"
    "src/*.vsh"
    "src/*.h.in"
    "src/imgui/*.h"
    "src/imgui/*.cpp"
    "src/hlp/*.cc"
    "src/hlp/*.hh"
    "src/hlp/render/*.cc"
    "src/hlp/render/*.hh"
    "src/hlp/app/*.cc"
    "src/hlp/app/*.hh"
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCES})

# ===============================================
# Make executable
add_executable(${PROJECT_NAME} ${SOURCES}) # for winmain add_executable(${PROJECT_NAME} WIN32 ${SOURCES})

# ===============================================
# Link Libs
target_link_libraries(${PROJECT_NAME} PRIVATE "d3d11.lib" "d3dcompiler.lib")

# ===============================================
# Include dirs
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/extern/dxsdk/Include")
