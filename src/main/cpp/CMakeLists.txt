cmake_minimum_required(VERSION 3.4.1)
set(NODE_LIB_DIR ${PROJECT_SOURCE_DIR}/prebuilts/${ANDROID_ABI})

set(SOURCES
        __onload.cpp
        EnvHelper.cpp
        NodeRuntime.cpp
        observable/PropertiesObserver.cpp
        types/JSContext.cpp
        types/JSValue.cpp
        types/JSObject.cpp
        types/JSUndefined.cpp
        types/JSBoolean.cpp
        types/JSNumber.cpp
        types/JSString.cpp
        types/JSArray.cpp
        types/JSNull.cpp
        types/JSFunction.cpp
        types/JavaCallback.cpp
        types/JSError.cpp
        types/JSPromise.cpp)

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

add_library(libnode SHARED IMPORTED)
set_target_properties(libnode PROPERTIES IMPORTED_LOCATION ${NODE_LIB_DIR}/lib/libnode.so)

find_library(log-lib log)
find_library(z-lib z)

target_link_libraries(knode libnode ${log-lib} ${z-lib})

add_executable(libnode_bin.so ./main.cpp)
target_link_libraries(libnode_bin.so libnode ${log-lib})

add_library(exec SHARED ./exec.c)
target_link_libraries(exec ${log-lib})

