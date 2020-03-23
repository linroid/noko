cmake_minimum_required(VERSION 3.4.1)
add_definitions()
#set(JS_GENERATE_DIR ${PROJECT_SOURCE_DIR}/../../../build/generated/node/${ANDROID_ABI}/)

#set(NODE_SOURCE ${PROJECT_SOURCE_DIR}/../../../../deps/node)
#set(RUNTIME_SOURCE ${PROJECT_SOURCE_DIR}/../../../../js/dist)

#set(UV_SOURCE ${NODE_SOURCE}/deps/uv)
#set(CARES_SOURCE ${NODE_SOURCE}/deps/cares)
#set(NGHTTP2_SOURCE ${NODE_SOURCE}/deps/nghttp2)
#set(OPENSSL_SOURCE ${NODE_SOURCE}/deps/openssl/openssl)

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
#        OUTPUT ${JS_GENERATE_DIR}/node_javascript.cc
#        DEPENDS ${NODE_SOURCE}/lib/internal/per_context.js ${NODE_SOURCE}/lib/internal/bootstrap/cache.js ${NODE_SOURCE}/lib/internal/bootstrap/loaders.js ${NODE_SOURCE}/lib/internal/bootstrap/node.js ${NODE_SOURCE}/lib/async_hooks.js ${NODE_SOURCE}/lib/assert.js ${NODE_SOURCE}/lib/buffer.js ${NODE_SOURCE}/lib/child_process.js ${NODE_SOURCE}/lib/console.js ${NODE_SOURCE}/lib/constants.js ${NODE_SOURCE}/lib/crypto.js ${NODE_SOURCE}/lib/cluster.js ${NODE_SOURCE}/lib/dgram.js ${NODE_SOURCE}/lib/dns.js ${NODE_SOURCE}/lib/domain.js ${NODE_SOURCE}/lib/events.js ${NODE_SOURCE}/lib/fs.js ${NODE_SOURCE}/lib/http.js ${NODE_SOURCE}/lib/http2.js ${NODE_SOURCE}/lib/_http_agent.js ${NODE_SOURCE}/lib/_http_client.js ${NODE_SOURCE}/lib/_http_common.js ${NODE_SOURCE}/lib/_http_incoming.js ${NODE_SOURCE}/lib/_http_outgoing.js ${NODE_SOURCE}/lib/_http_server.js ${NODE_SOURCE}/lib/https.js ${NODE_SOURCE}/lib/inspector.js ${NODE_SOURCE}/lib/module.js ${NODE_SOURCE}/lib/net.js ${NODE_SOURCE}/lib/os.js ${NODE_SOURCE}/lib/path.js ${NODE_SOURCE}/lib/perf_hooks.js ${NODE_SOURCE}/lib/process.js ${NODE_SOURCE}/lib/punycode.js ${NODE_SOURCE}/lib/querystring.js ${NODE_SOURCE}/lib/readline.js ${NODE_SOURCE}/lib/repl.js ${NODE_SOURCE}/lib/stream.js ${NODE_SOURCE}/lib/_stream_readable.js ${NODE_SOURCE}/lib/_stream_writable.js ${NODE_SOURCE}/lib/_stream_duplex.js ${NODE_SOURCE}/lib/_stream_transform.js ${NODE_SOURCE}/lib/_stream_passthrough.js ${NODE_SOURCE}/lib/_stream_wrap.js ${NODE_SOURCE}/lib/string_decoder.js ${NODE_SOURCE}/lib/sys.js ${NODE_SOURCE}/lib/timers.js ${NODE_SOURCE}/lib/tls.js ${NODE_SOURCE}/lib/_tls_common.js ${NODE_SOURCE}/lib/_tls_wrap.js ${NODE_SOURCE}/lib/trace_events.js ${NODE_SOURCE}/lib/tty.js ${NODE_SOURCE}/lib/url.js ${NODE_SOURCE}/lib/util.js ${NODE_SOURCE}/lib/v8.js ${NODE_SOURCE}/lib/vm.js ${NODE_SOURCE}/lib/worker_threads.js ${NODE_SOURCE}/lib/zlib.js ${NODE_SOURCE}/lib/internal/assert.js ${NODE_SOURCE}/lib/internal/async_hooks.js ${NODE_SOURCE}/lib/internal/bash_completion.js ${NODE_SOURCE}/lib/internal/buffer.js ${NODE_SOURCE}/lib/internal/cli_table.js ${NODE_SOURCE}/lib/internal/child_process.js ${NODE_SOURCE}/lib/internal/cluster/child.js ${NODE_SOURCE}/lib/internal/cluster/master.js ${NODE_SOURCE}/lib/internal/cluster/round_robin_handle.js ${NODE_SOURCE}/lib/internal/cluster/shared_handle.js ${NODE_SOURCE}/lib/internal/cluster/utils.js ${NODE_SOURCE}/lib/internal/cluster/worker.js ${NODE_SOURCE}/lib/internal/crypto/certificate.js ${NODE_SOURCE}/lib/internal/crypto/cipher.js ${NODE_SOURCE}/lib/internal/crypto/diffiehellman.js ${NODE_SOURCE}/lib/internal/crypto/hash.js ${NODE_SOURCE}/lib/internal/crypto/keygen.js ${NODE_SOURCE}/lib/internal/crypto/pbkdf2.js ${NODE_SOURCE}/lib/internal/crypto/random.js ${NODE_SOURCE}/lib/internal/crypto/scrypt.js ${NODE_SOURCE}/lib/internal/crypto/sig.js ${NODE_SOURCE}/lib/internal/crypto/util.js ${NODE_SOURCE}/lib/internal/constants.js ${NODE_SOURCE}/lib/internal/dgram.js ${NODE_SOURCE}/lib/internal/dns/promises.js ${NODE_SOURCE}/lib/internal/dns/utils.js ${NODE_SOURCE}/lib/internal/domexception.js ${NODE_SOURCE}/lib/internal/encoding.js ${NODE_SOURCE}/lib/internal/errors.js ${NODE_SOURCE}/lib/internal/error-serdes.js ${NODE_SOURCE}/lib/internal/fixed_queue.js ${NODE_SOURCE}/lib/internal/freelist.js ${NODE_SOURCE}/lib/internal/fs/promises.js ${NODE_SOURCE}/lib/internal/fs/read_file_context.js ${NODE_SOURCE}/lib/internal/fs/streams.js ${NODE_SOURCE}/lib/internal/fs/sync_write_stream.js ${NODE_SOURCE}/lib/internal/fs/utils.js ${NODE_SOURCE}/lib/internal/fs/watchers.js ${NODE_SOURCE}/lib/internal/http.js ${NODE_SOURCE}/lib/internal/inspector_async_hook.js ${NODE_SOURCE}/lib/internal/linkedlist.js ${NODE_SOURCE}/lib/internal/modules/cjs/helpers.js ${NODE_SOURCE}/lib/internal/modules/cjs/loader.js ${NODE_SOURCE}/lib/internal/modules/esm/loader.js ${NODE_SOURCE}/lib/internal/modules/esm/create_dynamic_module.js ${NODE_SOURCE}/lib/internal/modules/esm/default_resolve.js ${NODE_SOURCE}/lib/internal/modules/esm/module_job.js ${NODE_SOURCE}/lib/internal/modules/esm/module_map.js ${NODE_SOURCE}/lib/internal/modules/esm/translators.js ${NODE_SOURCE}/lib/internal/safe_globals.js ${NODE_SOURCE}/lib/internal/net.js ${NODE_SOURCE}/lib/internal/options.js ${NODE_SOURCE}/lib/internal/print_help.js ${NODE_SOURCE}/lib/internal/process/esm_loader.js ${NODE_SOURCE}/lib/internal/process/main_thread_only.js ${NODE_SOURCE}/lib/internal/process/next_tick.js ${NODE_SOURCE}/lib/internal/process/per_thread.js ${NODE_SOURCE}/lib/internal/process/promises.js ${NODE_SOURCE}/lib/internal/process/stdio.js ${NODE_SOURCE}/lib/internal/process/warning.js ${NODE_SOURCE}/lib/internal/process/worker_thread_only.js ${NODE_SOURCE}/lib/internal/querystring.js ${NODE_SOURCE}/lib/internal/process/write-coverage.js ${NODE_SOURCE}/lib/internal/process/coverage.js ${NODE_SOURCE}/lib/internal/readline.js ${NODE_SOURCE}/lib/internal/repl.js ${NODE_SOURCE}/lib/internal/repl/await.js ${NODE_SOURCE}/lib/internal/repl/recoverable.js ${NODE_SOURCE}/lib/internal/socket_list.js ${NODE_SOURCE}/lib/internal/test/binding.js ${NODE_SOURCE}/lib/internal/test/heap.js ${NODE_SOURCE}/lib/internal/test/unicode.js ${NODE_SOURCE}/lib/internal/timers.js ${NODE_SOURCE}/lib/internal/tls.js ${NODE_SOURCE}/lib/internal/trace_events_async_hooks.js ${NODE_SOURCE}/lib/internal/tty.js ${NODE_SOURCE}/lib/internal/url.js ${NODE_SOURCE}/lib/internal/util.js ${NODE_SOURCE}/lib/internal/util/comparisons.js ${NODE_SOURCE}/lib/internal/util/inspect.js ${NODE_SOURCE}/lib/internal/util/inspector.js ${NODE_SOURCE}/lib/internal/util/types.js ${NODE_SOURCE}/lib/internal/http2/core.js ${NODE_SOURCE}/lib/internal/http2/compat.js ${NODE_SOURCE}/lib/internal/http2/util.js ${NODE_SOURCE}/lib/internal/v8_prof_polyfill.js ${NODE_SOURCE}/lib/internal/v8_prof_processor.js ${NODE_SOURCE}/lib/internal/validators.js ${NODE_SOURCE}/lib/internal/stream_base_commons.js ${NODE_SOURCE}/lib/internal/vm/source_text_module.js ${NODE_SOURCE}/lib/internal/worker.js ${NODE_SOURCE}/lib/internal/streams/lazy_transform.js ${NODE_SOURCE}/lib/internal/streams/async_iterator.js ${NODE_SOURCE}/lib/internal/streams/buffer_list.js ${NODE_SOURCE}/lib/internal/streams/duplexpair.js ${NODE_SOURCE}/lib/internal/streams/legacy.js ${NODE_SOURCE}/lib/internal/streams/destroy.js ${NODE_SOURCE}/lib/internal/streams/state.js ${NODE_SOURCE}/lib/internal/streams/pipeline.js ${NODE_SOURCE}/lib/internal/streams/end-of-stream.js ${NODE_SOURCE}/lib/internal/wrap_js_stream.js ${NODE_SOURCE}/deps/v8/tools/splaytree.js ${NODE_SOURCE}/deps/v8/tools/codemap.js ${NODE_SOURCE}/deps/v8/tools/consarray.js ${NODE_SOURCE}/deps/v8/tools/csvparser.js ${NODE_SOURCE}/deps/v8/tools/profile.js ${NODE_SOURCE}/deps/v8/tools/profile_view.js ${NODE_SOURCE}/deps/v8/tools/logreader.js ${NODE_SOURCE}/deps/v8/tools/arguments.js ${NODE_SOURCE}/deps/v8/tools/tickprocessor.js ${NODE_SOURCE}/deps/v8/tools/SourceMap.js ${NODE_SOURCE}/deps/v8/tools/tickprocessor-driver.js ${NODE_SOURCE}/deps/node-inspect/lib/_inspect.js ${NODE_SOURCE}/deps/node-inspect/lib/internal/inspect_client.js ${NODE_SOURCE}/deps/node-inspect/lib/internal/inspect_repl.js ${NODE_SOURCE}/deps/acorn/dist/acorn.js ${NODE_SOURCE}/deps/acorn/dist/walk.js ${PROJECT_SOURCE_DIR}/config/${ANDROID_ABI}/config.gypi ${NODE_SOURCE}/tools/check_macros.py ${NODE_SOURCE}/src/notrace_macros.py ${NODE_SOURCE}/src/noperfctr_macros.py ${NODE_SOURCE}/tools/nodcheck_macros.py ${NODE_SOURCE}/lib/__fs.js ${RUNTIME_SOURCE}/dora.js ${RUNTIME_SOURCE}/yarn.js ${RUNTIME_SOURCE}/router.js
#        WORKING_DIRECTORY ${NODE_SOURCE}
#        COMMAND mkdir -p ${JS_GENERATE_DIR}
#        COMMAND cp ${PROJECT_SOURCE_DIR}/config/${ANDROID_ABI}/config.gypi ${NODE_SOURCE}
#        COMMAND cp -r ${RUNTIME_SOURCE}/*js ${NODE_SOURCE}/lib/
#        COMMAND python tools/js2c.py "${JS_GENERATE_DIR}/node_javascript.cc" lib/internal/per_context.js lib/internal/bootstrap/cache.js lib/internal/bootstrap/loaders.js lib/internal/bootstrap/node.js lib/async_hooks.js lib/assert.js lib/buffer.js lib/child_process.js lib/console.js lib/constants.js lib/crypto.js lib/cluster.js lib/dgram.js lib/dns.js lib/domain.js lib/events.js lib/fs.js lib/http.js lib/http2.js lib/_http_agent.js lib/_http_client.js lib/_http_common.js lib/_http_incoming.js lib/_http_outgoing.js lib/_http_server.js lib/https.js lib/inspector.js lib/module.js lib/net.js lib/os.js lib/path.js lib/perf_hooks.js lib/process.js lib/punycode.js lib/querystring.js lib/readline.js lib/repl.js lib/stream.js lib/_stream_readable.js lib/_stream_writable.js lib/_stream_duplex.js lib/_stream_transform.js lib/_stream_passthrough.js lib/_stream_wrap.js lib/string_decoder.js lib/sys.js lib/timers.js lib/tls.js lib/_tls_common.js lib/_tls_wrap.js lib/trace_events.js lib/tty.js lib/url.js lib/util.js lib/v8.js lib/vm.js lib/worker_threads.js lib/zlib.js lib/internal/assert.js lib/internal/async_hooks.js lib/internal/bash_completion.js lib/internal/buffer.js lib/internal/cli_table.js lib/internal/child_process.js lib/internal/cluster/child.js lib/internal/cluster/master.js lib/internal/cluster/round_robin_handle.js lib/internal/cluster/shared_handle.js lib/internal/cluster/utils.js lib/internal/cluster/worker.js lib/internal/crypto/certificate.js lib/internal/crypto/cipher.js lib/internal/crypto/diffiehellman.js lib/internal/crypto/hash.js lib/internal/crypto/keygen.js lib/internal/crypto/pbkdf2.js lib/internal/crypto/random.js lib/internal/crypto/scrypt.js lib/internal/crypto/sig.js lib/internal/crypto/util.js lib/internal/constants.js lib/internal/dgram.js lib/internal/dns/promises.js lib/internal/dns/utils.js lib/internal/domexception.js lib/internal/encoding.js lib/internal/errors.js lib/internal/error-serdes.js lib/internal/fixed_queue.js lib/internal/freelist.js lib/internal/fs/promises.js lib/internal/fs/read_file_context.js lib/internal/fs/streams.js lib/internal/fs/sync_write_stream.js lib/internal/fs/utils.js lib/internal/fs/watchers.js lib/internal/http.js lib/internal/inspector_async_hook.js lib/internal/linkedlist.js lib/internal/modules/cjs/helpers.js lib/internal/modules/cjs/loader.js lib/internal/modules/esm/loader.js lib/internal/modules/esm/create_dynamic_module.js lib/internal/modules/esm/default_resolve.js lib/internal/modules/esm/module_job.js lib/internal/modules/esm/module_map.js lib/internal/modules/esm/translators.js lib/internal/safe_globals.js lib/internal/net.js lib/internal/options.js lib/internal/print_help.js lib/internal/process/esm_loader.js lib/internal/process/main_thread_only.js lib/internal/process/next_tick.js lib/internal/process/per_thread.js lib/internal/process/promises.js lib/internal/process/stdio.js lib/internal/process/warning.js lib/internal/process/worker_thread_only.js lib/internal/querystring.js lib/internal/process/write-coverage.js lib/internal/process/coverage.js lib/internal/readline.js lib/internal/repl.js lib/internal/repl/await.js lib/internal/repl/recoverable.js lib/internal/socket_list.js lib/internal/test/binding.js lib/internal/test/heap.js lib/internal/test/unicode.js lib/internal/timers.js lib/internal/tls.js lib/internal/trace_events_async_hooks.js lib/internal/tty.js lib/internal/url.js lib/internal/util.js lib/internal/util/comparisons.js lib/internal/util/inspect.js lib/internal/util/inspector.js lib/internal/util/types.js lib/internal/http2/core.js lib/internal/http2/compat.js lib/internal/http2/util.js lib/internal/v8_prof_polyfill.js lib/internal/v8_prof_processor.js lib/internal/validators.js lib/internal/stream_base_commons.js lib/internal/vm/source_text_module.js lib/internal/worker.js lib/internal/streams/lazy_transform.js lib/internal/streams/async_iterator.js lib/internal/streams/buffer_list.js lib/internal/streams/duplexpair.js lib/internal/streams/legacy.js lib/internal/streams/destroy.js lib/internal/streams/state.js lib/internal/streams/pipeline.js lib/internal/streams/end-of-stream.js lib/internal/wrap_js_stream.js deps/v8/tools/splaytree.js deps/v8/tools/codemap.js deps/v8/tools/consarray.js deps/v8/tools/csvparser.js deps/v8/tools/profile.js deps/v8/tools/profile_view.js deps/v8/tools/logreader.js deps/v8/tools/arguments.js deps/v8/tools/tickprocessor.js deps/v8/tools/SourceMap.js deps/v8/tools/tickprocessor-driver.js deps/node-inspect/lib/_inspect.js deps/node-inspect/lib/internal/inspect_client.js deps/node-inspect/lib/internal/inspect_repl.js deps/acorn/dist/acorn.js deps/acorn/dist/walk.js config.gypi tools/check_macros.py src/notrace_macros.py src/noperfctr_macros.py tools/nodcheck_macros.py lib/__fs.js lib/dora.js lib/yarn.js lib/router.js
#)

add_library(knode SHARED ${SOURCES})
#
#set(V8_PREBUILD_DIR ${PROJECT_SOURCE_DIR}/prebuilt)
#
#add_library(v8_base STATIC IMPORTED)
#set_target_properties(v8_base PROPERTIES IMPORTED_LOCATION ${V8_PREBUILD_DIR}/${ANDROID_ABI}/libv8_base.a)
#
#add_library(v8_libbase STATIC IMPORTED)
#set_target_properties(v8_libbase PROPERTIES IMPORTED_LOCATION ${V8_PREBUILD_DIR}/${ANDROID_ABI}/libv8_libbase.a)
#
#add_library(v8_snapshot STATIC IMPORTED)
#set_target_properties(v8_snapshot PROPERTIES IMPORTED_LOCATION ${V8_PREBUILD_DIR}/${ANDROID_ABI}/libv8_snapshot.a)
#
#add_library(v8_libsampler STATIC IMPORTED)
#set_target_properties(v8_libsampler PROPERTIES IMPORTED_LOCATION ${V8_PREBUILD_DIR}/${ANDROID_ABI}/libv8_libsampler.a)
#
#add_library(v8_libplatform STATIC IMPORTED)
#set_target_properties(v8_libplatform PROPERTIES IMPORTED_LOCATION ${V8_PREBUILD_DIR}/${ANDROID_ABI}/libv8_libplatform.a)
#
#if (${ANDROID_ABI} STREQUAL armeabi-v7a)
#    SET(SSL_ARCH "linux-armv4")
#    SET(NODE_ARCH "armv7a")
#elseif (${ANDROID_ABI} STREQUAL x86)
#    SET(SSL_ARCH "linux-x32")
#    SET(NODE_ARCH "x86")
#elseif (${ANDROID_ABI} STREQUAL x86_64)
#    SET(SSL_ARCH "linux-x86_64")
#    SET(NODE_ARCH "x64")
#elseif (${ANDROID_ABI} STREQUAL arm64-v8a)
#    SET(SSL_ARCH "linux-aarch64")
#    SET(NODE_ARCH "arm64")
#endif ()
#
#include_directories(
#        ${NODE_SOURCE}/deps/v8
#        ${NODE_SOURCE}/deps/v8/include
#        ${NODE_SOURCE}/deps/uv/src
#        ${NODE_SOURCE}/deps/uv/include
#        ${NODE_SOURCE}/deps/cares/src
#        ${NODE_SOURCE}/deps/cares/config/android
#        ${NODE_SOURCE}/deps/cares/include
#        ${NODE_SOURCE}/deps/http_parser
#        ${NODE_SOURCE}/deps/nghttp2/lib/includes
#        ${NODE_SOURCE}/src
#        ${NODE_SOURCE}/deps/openssl/openssl/include
#        ${NODE_SOURCE}/deps/openssl/openssl
#        ${NODE_SOURCE}/deps/openssl/openssl/include
#        ${NODE_SOURCE}/deps/openssl/openssl/crypto
#        ${NODE_SOURCE}/deps/openssl/openssl/crypto/include
#        ${NODE_SOURCE}/deps/openssl/openssl/crypto/modes
#        ${NODE_SOURCE}/deps/openssl/config
#        ${NODE_SOURCE}/deps/openssl/config/archs/${SSL_ARCH}/no-asm
#        ${NODE_SOURCE}/deps/openssl/config/archs/${SSL_ARCH}/no-asm/include
#        ${NODE_SOURCE}/deps/openssl/config/archs/${SSL_ARCH}/no-asm/crypto
#        ${NODE_SOURCE}/deps/openssl/config/archs/${SSL_ARCH}/no-asm/crypto/include/internal
#)
#unset(DEFS_RELEASE)
#unset(CFLAGS_RELEASE)
#unset(CPPFLAGS_RELEASE)
#list(APPEND DEFS_RELEASE
#        -DV8_INSPECTOR_USE_STL=1
#        -DV8_INSPECTOR_USE_OLD_STL=1
#        -DNODE_PLATFORM='"android"'
#        -DNODE_ARCH='"${NODE_ARCH}"'
#        -DNODE_WANT_INTERNALS=1
#        -DV8_DEPRECATION_WARNINGS=1
#        -DNODE_OPENSSL_SYSTEM_CERT_PATH='""'
#        -DHAVE_INSPECTOR=0
#        -DNODE_SHARED_MODE
#        -DNODE_USE_V8_PLATFORM=1
#        -DHAVE_OPENSSL=1
#        -DHTTP_PARSER_STRICT=0
#
#        -D__POSIX__
#        -D_REENTRANT=1
#        -D_DARWIN_USE_64_BIT_INODE=1
#        -D_LARGEFILE_SOURCE
#        -D_FILE_OFFSET_BITS=64
#        -D_GNU_SOURCE
#        -DHAVE_CONFIG_H
#        -D_GLIBCXX_USE_C99_MATH
#        -DANDROID_STL=c++_static
#
#        -DHTTP_MAX_HEADER_SIZE=8192
#        -DHTTP_PARSER_STRICT=0
#
#        -D_U_=
#        -DBUILDING_NGHTTP2
#        -DNGHTTP2_STATICLIB
#
#        -DOPENSSL_NO_HW
#        -DOPENSSL_NO_ASM
#        -DDSO_DLFCN
#        -DHAVE_DLFCN_H
#        -DOPENSSL_THREADS
#        -DOPENSSL_NO_DYNAMIC_ENGINE
#        -DOPENSSL_PIC
#        -DOPENSSLDIR='"/etc/ssl"'
#        -DENGINESDIR='"/dev/null"'
#        -DTERMIOS
#        )
#
#list(APPEND CFLAGS_RELEASE
#        -Wall
#        -Wextra
#        -Wno-unused-parameter
#        -fPIC
#        -O3
#        -fno-omit-frame-pointer
#        -fPIE
#        -Wno-strict-aliasing
#        -Wno-unused-variable
#        -Wno-mismatched-tags
#        -Wno-unused-result
#        -Wno-missing-field-initializers
#        )
#
#list(APPEND CPPFLAGS_RELEASE
#        -fno-rtti
#        -std=c++1y
#        )
#
#string(REPLACE ";" " " C_FLAGS_STR "${CFLAGS_RELEASE} ${DEFS_RELEASE}")
#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${C_FLAGS_STR}")
#
#string(REPLACE ";" " " CPP_FLAGS_STR "${CFLAGS_RELEASE} ${CPPFLAGS_RELEASE} ${DEFS_RELEASE}")
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CPP_FLAGS_STR}")

#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

include_directories(/Users/linroid/Sources/node/out/prefix/include/node/)

add_library(libnode
        SHARED
        IMPORTED)
set_target_properties( # Specifies the target library.
        libnode
        # Specifies the parameter you want to define.
        PROPERTIES IMPORTED_LOCATION
        # Provides the path to the library you want to import.
        /Users/linroid/Sources/node/out/prefix/lib/libnode.so)

find_library(log-lib log)
find_library(z-lib z)

target_link_libraries(knode libnode ${log-lib} ${z-lib})
