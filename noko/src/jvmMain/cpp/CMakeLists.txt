cmake_minimum_required(VERSION 3.4.1)
project(libnoko)
set(CMAKE_CXX_STANDARD 11)
if (ANDROID)
    set(TARGET_OS android)
    if (ANDROID_ABI STREQUAL arm64-v8a)
        set(TARGET_ARCH arm64)
    elseif (ANDROID_ABI STREQUAL armeabi-v7a)
        set(TARGET_ARCH arm)
    elseif (ANDROID_ABI STREQUAL x86_64)
        set(TARGET_ARCH x64)
    elseif (ANDROID_ABI STREQUAL x86)
        set(TARGET_ARCH x86)
    else ()
        message(FATAL_ERROR "Unexpected ANDROID_ABI: ${ANDROID_ABI}")
    endif ()
endif ()

set(LIBNODE_DIR ${PROJECT_SOURCE_DIR}/prebuilt/${TARGET_OS}/${TARGET_ARCH})
message(STATUS LIBNODE_DIR: ${LIBNODE_DIR})
message(STATUS TARGET_ARCH: ${TARGET_ARCH})
message(STATUS TARGET_OS: ${TARGET_OS})

set(SOURCES
        __onload.cpp
        util/env_helper.cpp
        util/jni_helper.cpp
        runtime.cpp
        observable/properties_observer.cpp
        types/js_value.cpp
        types/js_object.cpp
        types/js_undefined.cpp
        types/boolean.cpp
        types/double.cpp
        types/string.cpp
        types/integer.cpp
        types/long.cpp
        types/js_array.cpp
        types/js_function.cpp
        util/java_callback.cpp
        types/js_error.cpp
        types/js_promise.cpp)

add_library(noko SHARED ${SOURCES})
include_directories(${LIBNODE_DIR}/include/node/)
if (ANDROID)
    add_library(node SHARED IMPORTED)
    set_target_properties(node PROPERTIES IMPORTED_LOCATION ${LIBNODE_DIR}/lib/libnode.so)

    find_library(log-lib log)
    target_link_libraries(noko node ${log-lib})
else ()
    find_library(node-lib
            NAMES node node.93
            HINTS ${LIBNODE_DIR}/lib
            )
    find_package(JNI REQUIRED)
    include_directories(${JNI_INCLUDE_DIRS})
    target_link_libraries(noko ${node-lib})
endif ()

