cmake_minimum_required(VERSION 3.4.1)
#add_definitions()
set(NODE_LIB_DIR ${PROJECT_SOURCE_DIR}/../prebuilts/${ANDROID_ABI})
set(NODE_SOURCE_DIR ${PROJECT_SOURCE_DIR}/../../../../deps/node/deps/cares/include)
#set(NODE_BUILD_DIR ${PROJECT_SOURCE_DIR}/prebuilt/${ANDROID_ABI}/prefix)
#set(LIB_NODE_OUTPUT ${PROJECT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libnode.so)
#set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../resources/lib/${ANDROID_ABI}")

set(SOURCES
        __onload.cpp
        base64.cpp
        NodeRuntime.cpp
        jni/JSContext.cpp
        jni/JSValue.cpp
        jni/JSObject.cpp
        jni/JSUndefined.cpp
        jni/JSBoolean.cpp
        jni/JSNumber.cpp
        jni/JSString.cpp
        jni/JSArray.cpp
        jni/JSNull.cpp
        jni/JSFunction.cpp
        jni/JniCallback.cpp
        jni/JSError.cpp
        jni/JSPromise.cpp)

#add_custom_command(
#        OUTPUT ${LIB_NODE_OUTPUT}
#        DEPENDS ${NODE_BUILD_DIR}/lib/libnode.so
#        WORKING_DIRECTORY ${NODE_BUILD_DIR}
#        COMMAND cp ${NODE_BUILD_DIR}/lib/libnode.so ${LIB_NODE_OUTPUT}
#)

add_library(knode SHARED ${SOURCES})

get_cmake_property(_variableNames VARIABLES)
list(SORT _variableNames)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach ()

include_directories(${NODE_LIB_DIR}/include/node/)
include_directories(${NODE_SOURCE_DIR})

add_library(libnode SHARED IMPORTED)
set_target_properties( # Specifies the target library.
        libnode
        # Specifies the parameter you want to define.
        PROPERTIES IMPORTED_LOCATION
        # Provides the path to the library you want to import.
        ${NODE_LIB_DIR}/libnode.so)

find_library(log-lib log)
find_library(z-lib z)

target_link_libraries(knode libnode ${log-lib} ${z-lib})

add_executable(node.so ./main.cpp)
target_link_libraries(node.so libnode ${log-lib})

add_library(exec SHARED ./exec.c)
target_link_libraries(exec ${log-lib})

