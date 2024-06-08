cmake_minimum_required(VERSION 3.23)
set(CMAKE_OSX_SYSROOT, "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX11.0.sdk")
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0")
project(Client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_OSX_ARCHITECTURES "x86_64")
set(CMAKE_CXX_FLAGS "-stdlib=libc++ -std=c++20 -lz -framework Security -framework Foundation -dynamiclib -Qunused-arguments -lcurl")

find_library(APPLE_FWK_FOUNDATION Foundation REQUIRED)
find_library(APPLE_FWK_QUARTZ_CORE QuartzCore REQUIRED)
find_library(APPLE_FWK_CORE_GRAPHICS CoreGraphics REQUIRED)
find_library(APPLE_FWK_METAL Metal REQUIRED)
find_library(APPLE_FWK_METAL_KIT MetalKit REQUIRED)
find_library(APPLE_FWK_APP_KIT AppKit REQUIRED)

include_directories(.)

add_executable(Client
        exploit.cpp functions.h http_request.h communication.h)

target_link_libraries(Client /Applications/Roblox.app/Contents/MacOS/libdiscord-rpc.dylib)
target_link_libraries(Client ${CMAKE_SOURCE_DIR}/ixwebsocket/libixwebsocket.a)
target_link_libraries(Client ${CMAKE_SOURCE_DIR}/LuauLibraries/libLuau.Ast.a)
target_link_libraries(Client ${CMAKE_SOURCE_DIR}/LuauLibraries/libLuau.Compiler.a)
target_link_libraries(Client ${CMAKE_SOURCE_DIR}/openssl/libcrypto.a)
target_link_libraries(Client
        ${APPLE_FWK_FOUNDATION}
        ${APPLE_FWK_QUARTZ_CORE}
        ${APPLE_FWK_METAL}
        ${APPLE_FWK_CORE_GRAPHICS}
        ${APPLE_FWK_METAL_KIT}
        ${APPLE_FWK_APP_KIT}
)

set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_metal.mm PROPERTIES COMPILE_FLAGS -fobjc-weak)
target_include_directories(Client PRIVATE ${CMAKE_SOURCE_DIR}/ixwebsocket)
target_include_directories(Client PRIVATE ${CMAKE_SOURCE_DIR}/openssl)
target_include_directories(Client PRIVATE ${CMAKE_SOURCE_DIR}/Luau)
target_include_directories(Client PRIVATE ${CMAKE_SOURCE_DIR}/imgui)
target_include_directories(Client
  SYSTEM PUBLIC ${CMAKE_SOURCE_DIR}/metal-cpp
)

set_target_properties(
        Client
        PROPERTIES
        OUTPUT_NAME "exploit"
        SUFFIX ".dylib"
)