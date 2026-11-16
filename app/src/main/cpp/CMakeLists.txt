cmake_minimum_required(VERSION 3.22.1)
project(mobileblox_luau CXX C)

set(LUAU_BUILD_CLI OFF CACHE BOOL "" FORCE)
set(LUAU_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(LUAU_BUILD_WEB OFF CACHE BOOL "" FORCE)
set(LUAU_WERROR OFF CACHE BOOL "" FORCE)

add_subdirectory(luau EXCLUDE_FROM_ALL)

add_library(mobileblox_luau SHARED luau_jni.cpp)

target_compile_features(mobileblox_luau PRIVATE cxx_std_17)

target_link_libraries(mobileblox_luau PRIVATE
    Luau.Compiler
    Luau.VM
    Luau.Ast
    Luau.Common
    log
)

target_include_directories(mobileblox_luau PRIVATE
    luau/Common/include
    luau/Ast/include
    luau/Compiler/include
    luau/VM/include
)
