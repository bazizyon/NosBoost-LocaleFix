cmake_minimum_required(VERSION 4.1)
project(LocaleFix)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG 691b89baae95d56df42a59b031c9d19685b7ab2f
)
FetchContent_MakeAvailable(imgui)

add_library(LocaleFix SHARED
        LocaleFix/LocaleFix.cpp
)

target_include_directories(LocaleFix PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${imgui_SOURCE_DIR}
)

target_compile_options(LocaleFix PRIVATE /EHsc /MP)

if(NOT DEFINED ENV{CI})
    set_target_properties(LocaleFix PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "C:/Program Files (x86)/Nostale/mods"
    )
endif()
