cmake_minimum_required(VERSION 3.4.1)
project(libnoko)
set(CMAKE_CXX_STANDARD 11)
if (ANDROID)
    set(TARGET_OS android)
    set(TARGET_ARCH ${ANDROID_ABI})
    set(LIBNODE_NAME libnode.so)
elseif (CMAKE_SYSTEM_NAME STREQUAL Linux)
    set(TARGET_OS linux)
    set(TARGET_ARCH ${CMAKE_HOST_SYSTEM_PROCESSOR})
elseif (CMAKE_SYSTEM_NAME STREQUAL Darwin)
    set(TARGET_OS macos)
    set(TARGET_ARCH ${CMAKE_HOST_SYSTEM_PROCESSOR})
    set(LIBNODE_NAME libnode.93.dylib)
elseif (CMAKE_SYSTEM_NAME STREQUAL Windows)
    set(TARGET_OS windows)
    set(TARGET_ARCH ${CMAKE_HOST_SYSTEM_PROCESSOR})
else ()
    message(FATAL_ERROR "Not supported: system=${CMAKE_SYSTEM_NAME}, processor=${CMAKE_HOST_SYSTEM_PROCESSOR}")
endif ()

set(LIBNODE_DIR ${PROJECT_SOURCE_DIR}/prebuilt/${TARGET_OS}/${TARGET_ARCH})
message(LIBNODE_DIR: ${LIBNODE_DIR})

set(SOURCES
        __onload.cpp
        util/env_helper.cpp
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
        types/js_null.cpp
        types/js_function.cpp
        util/java_callback.cpp
        types/js_error.cpp
        types/js_promise.cpp)

add_library(noko SHARED ${SOURCES})

include_directories(${LIBNODE_DIR}/include/node/)

add_library(libnode SHARED IMPORTED)
set_target_properties(libnode PROPERTIES IMPORTED_LOCATION ${LIBNODE_DIR}/lib/${LIBNODE_NAME})

if (ANDROID)
    find_library(log-lib log)
    target_link_libraries(noko libnode ${log-lib})
else ()
    find_package(JNI REQUIRED)
    include_directories(${JNI_INCLUDE_DIRS})
    target_link_libraries(noko libnode)
endif ()

