cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0063 NEW)

project(flecs LANGUAGES C)

option(FLECS_STATIC "Build static flecs lib" ON)
option(FLECS_SHARED "Build shared flecs lib" ON)
option(FLECS_PIC "Compile static flecs lib with position independent code (PIC)" ON)
option(FLECS_TESTS "Build flecs tests" OFF)
option(BUILD_WASM_COMPONENT "Build the Flecs WASM component" OFF)

include(cmake/target_default_compile_warnings.cmake)
include(cmake/target_default_compile_options.cmake)

# Automatically generate the same folder structure in Visual Studio as we have on disk
macro(GroupSources curdir)
	file(GLOB children RELATIVE ${PROJECT_SOURCE_DIR}/${curdir} ${PROJECT_SOURCE_DIR}/${curdir}/*)
	foreach(child ${children})
		if(IS_DIRECTORY ${PROJECT_SOURCE_DIR}/${curdir}/${child})
			GroupSources(${curdir}/${child})
		else()
			string(REPLACE "/" "\\" groupname ${curdir})
            source_group(${groupname} FILES ${PROJECT_SOURCE_DIR}/${curdir}/${child})
		endif()
	endforeach()
endmacro()

file(GLOB children RELATIVE ${PROJECT_SOURCE_DIR}/. ${PROJECT_SOURCE_DIR}/./*)
foreach(child ${children})
	if(IS_DIRECTORY ${PROJECT_SOURCE_DIR}/${curdir}/${child})
		GroupSources(${child})
	endif()
endforeach()

file(GLOB_RECURSE INC include/*.h include/*.hpp)
file(GLOB_RECURSE SRC src/*.c)

set(FLECS_TARGETS "")

macro(add_flecs_target TARGET CONFIG)
    add_library(${TARGET} ${CONFIG} ${INC} ${SRC})
    add_library(flecs::${TARGET} ALIAS ${TARGET})

    target_default_compile_options_c(${TARGET})
    target_default_compile_warnings_c(${TARGET})
    if(WIN32)
        target_link_libraries(${TARGET} wsock32 ws2_32)
    endif()

    if(CMAKE_SYSTEM_NAME STREQUAL "Android")
        target_include_directories(${TARGET} PRIVATE  
        "${CMAKE_ANDROID_NDK}/toolchains/llvm/prebuilt/windows-x86_64/include/")  
        message(STATUS "building Android project...")
    endif()

    if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_link_libraries(${TARGET} pthread)
    endif()

    if(FLECS_PIC)
        set_property(TARGET ${TARGET} PROPERTY POSITION_INDEPENDENT_CODE ON)
    endif()

    target_include_directories(${TARGET} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

    list(APPEND FLECS_TARGETS ${TARGET})
endmacro()

if(FLECS_SHARED)
    add_flecs_target(flecs SHARED)
endif()

if(FLECS_STATIC)
    add_flecs_target(flecs_static STATIC)
    target_compile_definitions(flecs_static PUBLIC flecs_STATIC)
endif()

if(FLECS_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

if(BUILD_WASM_COMPONENT)
    message(STATUS "Standalone WebAssembly component build is enabled.")

    find_program(WIT_BINDGEN_CLI wit-bindgen REQUIRED)
    if(NOT WASI_SDK_PATH AND DEFINED ENV{WASI_SDK_ROOT})
        set(WASI_SDK_PATH "$ENV{WASI_SDK_ROOT}" CACHE PATH "Path to the WASI SDK root directory")
    endif()

    if(NOT WASI_SDK_PATH)
        message(FATAL_ERROR 
            "WASI SDK path not found. \n"
            "Please set the WASI_SDK_ROOT environment variable, \n"
            "or pass the path to CMake: -DWASI_SDK_PATH=/path/to/wasi-sdk"
        )
    endif()

    find_program(CC clang
        PATHS "${WASI_SDK_PATH}/bin"
        NO_DEFAULT_PATH
        REQUIRED
    )

    message(STATUS "Found WASI SDK at: ${WASI_SDK_PATH}")
    message(STATUS "Using WASI clang: ${CC}")

    set(WIT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/flecs.wit")
    set(WIT_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/wit_gen")
    set(WIT_GENERATED_C_SRC "${WIT_GEN_DIR}/flecs_world.c")
    set(WIT_GENERATED_H "${WIT_GEN_DIR}/flecs_world.h")
    set(WIT_GENERATED_COMPONENT_TYPE_OBJ "${WIT_GEN_DIR}/flecs_world_component_type.o")

    add_custom_command(
        OUTPUT ${WIT_GENERATED_C_SRC} ${WIT_GENERATED_H} ${WIT_GENERATED_COMPONENT_TYPE_OBJ}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${WIT_GEN_DIR}
        COMMAND ${WIT_BINDGEN_CLI} c --out-dir ${WIT_GEN_DIR} ${WIT_FILE}
        DEPENDS ${WIT_FILE}
        COMMENT "Generating bindings from flecs.wit"
    )
    add_custom_target(WitGen ALL DEPENDS ${WIT_GENERATED_C_SRC})

    set(WASM_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/distr/flecs.c
        ${CMAKE_CURRENT_SOURCE_DIR}/flecs_component.c
        ${WIT_GENERATED_C_SRC}
        ${WIT_GENERATED_COMPONENT_TYPE_OBJ}
    )

    set(WASM_CFLAGS
        "-g3"
        "-O0"
        "-fno-omit-frame-pointer"
        "-I${CMAKE_CURRENT_SOURCE_DIR}"
        "-I${CMAKE_CURRENT_SOURCE_DIR}/include"
        "-I${WIT_GEN_DIR}"
        "-U__STDC_LIB_EXT1__"
        "-DECS_TARGET_POSIX"
        "-DECS_TARGET_WASMTIME"
        "-DFLECS_CUSTOM_BUILD"
        "-DFLECS_META"
    )

    set(WASM_LDFLAGS
        "-mexec-model=reactor"
        "--target=wasm32-wasi"
        "-lwasi-emulated-pthread"
        "-Wl,--max-memory=16777216"
        "-Wl,--export-dynamic"
        "-Wl,--allow-undefined"
        "-Wl,--no-entry"
    )

    set(FINAL_WASM_TARGET "${CMAKE_CURRENT_BINARY_DIR}/flecs.wasm")
    add_custom_command(
        OUTPUT ${FINAL_WASM_TARGET}
        COMMAND ${CC} ${WASM_CFLAGS} ${WASM_SOURCES} -o ${FINAL_WASM_TARGET} ${WASM_LDFLAGS}
        DEPENDS ${WASM_SOURCES} WitGen
        COMMENT "Compiling and linking final component: flecs.wasm"
    )

    add_custom_target(flecs-component ALL DEPENDS ${FINAL_WASM_TARGET})

    message(STATUS "Configured 'flecs-component' target. Run 'ninja' or 'make' to build.")
endif()

message(STATUS "Targets: ${FLECS_TARGETS}")

# define the install steps
include(GNUInstallDirs)
install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
        PATTERN "*.inl")

install(TARGETS ${FLECS_TARGETS}
        EXPORT flecs-export
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(EXPORT flecs-export
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/flecs
        NAMESPACE flecs::
        FILE flecs-config.cmake)
