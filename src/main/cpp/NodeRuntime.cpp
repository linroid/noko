//
// Created by linroid on 2019-10-19.
//

#include <jni.h>
#include <mutex>
#include <uv.h>
#include <v8.h>
#include <libplatform/libplatform.h>
#include "NodeRuntime.h"
#include "jni/JSContext.h"
#include "jni/JSValue.h"
#include "jni/JSUndefined.h"
#include "jni/JSBoolean.h"
#include "jni/JSNumber.h"
#include "jni/JSObject.h"
#include "jni/JSString.h"
#include "jni/JSFunction.h"
#include "jni/JSNull.h"
#include "jni/JSPromise.h"
#include "jni/JSArray.h"
#include "jni/JSError.h"

int NodeRuntime::instanceCount = 0;
std::mutex NodeRuntime::sharedMutex;
int NodeRuntime::seq = 0;

void NodeRuntime::StaticOnPrepared(const v8::FunctionCallbackInfo<v8::Value> &info) {
    LOGD("StaticOnPrepared");
    NodeRuntime *instance = NodeRuntime::GetCurrent(info);
    instance->OnPrepared();
}

void NodeRuntime::StaticBeforeExit(const v8::FunctionCallbackInfo<v8::Value> &info) {
    auto context = info.GetIsolate()->GetCurrentContext();
    int code = info[0]->Int32Value(context).FromMaybe(0);
    LOGD("StaticBeforeExit: %d", code);
    // uv_walk(env->event_loop(), [](uv_handle_t *h, void *arg) {
    //     uv_unref(h);
    // }, nullptr);
}

void NodeRuntime::StaticHandle(uv_async_t *handle) {
    NodeRuntime *runtime = reinterpret_cast<NodeRuntime *>(handle->data);
    runtime->Handle(handle);
}

NodeRuntime::NodeRuntime(JNIEnv *env, jobject jThis, jmethodID onBeforeStart, jmethodID onBeforeExit)
        : onBeforeStart(onBeforeStart), onBeforeExit(onBeforeExit) {
    env->GetJavaVM(&_vm);
    this->jThis = env->NewGlobalRef(jThis);

    sharedMutex.lock();
    ++instanceCount;
    ++seq;
    id = seq;
    sharedMutex.unlock();
    LOGD("Construct NodeRuntime: thread_id=%d, id=%d, this=%p", std::this_thread::get_id(), id, this);
}

NodeRuntime::~NodeRuntime() {
    LOGE("~NodeRuntime(): thread_id=%d, id=%d, this=%p", std::this_thread::get_id(), id, this);
    std::lock_guard<std::mutex> instanceLock(instanceMutex);
    std::lock_guard<std::mutex> lock(asyncMutex);
    ENTER_JNI(_vm)
        env->DeleteGlobalRef(jThis);
    EXIT_JNI(_vm)

    uv_stop(_eventLoop);
    _eventLoop = nullptr;
    _global = nullptr;
    _isolate = nullptr;
    _running = false;
    sharedMutex.lock();
    --instanceCount;
    sharedMutex.unlock();
    LOGE("~NodeRuntime() finished");
}

void NodeRuntime::OnPrepared() {
    LOGI("OnPrepared");
    auto localGlobal = _global->Get(_isolate);
    auto localContext = _context.Get(_isolate);
    localGlobal->Delete(localContext, v8::String::NewFromUtf8(_isolate, "__onPrepared").ToLocalChecked());

    ENTER_JNI(_vm);
        env->CallVoidMethod(jThis, onBeforeStart, jContext);
    EXIT_JNI(_vm);
}

void NodeRuntime::SetUp() {
    LOGI("SetUp");

    v8::Local<v8::Context> context = _context.Get(_isolate);
    v8::HandleScope handleScope(_isolate);
    v8::Context::Scope contextScope(context);
    v8::Local<v8::Object> global = context->Global();
    _global = new v8::Persistent<v8::Object>(_isolate, global);

    _running = true;

    ENTER_JNI(_vm);
        auto nullValue = new v8::Persistent<v8::Value>(_isolate, v8::Null(_isolate));
        auto undefinedValue = new v8::Persistent<v8::Value>(_isolate, v8::Undefined(_isolate));
        auto trueValue = new v8::Persistent<v8::Value>(_isolate, v8::True(_isolate));
        auto falseValue = new v8::Persistent<v8::Value>(_isolate, v8::False(_isolate));
        this->jContext = env->NewGlobalRef(JSContext::Wrap(env, this));
        this->jNull = env->NewGlobalRef(JSNull::Wrap(env, this, nullValue));
        this->jUndefined = env->NewGlobalRef(JSUndefined::Wrap(env, this, undefinedValue));
        this->jTrue = env->NewGlobalRef(JSBoolean::Wrap(env, this, trueValue, true));
        this->jFalse = env->NewGlobalRef(JSBoolean::Wrap(env, this, falseValue, false));
        JSContext::SetShared(env, this);
    EXIT_JNI(_vm);

    // nodeEnv->SetMethod(process, "reallyExit", StaticBeforeExit);
    // nodeEnv->SetMethod(process, "abort", StaticBeforeExit);
    // nodeEnv->SetMethod(process, "_kill", StaticBeforeExit);

    auto data = v8::External::New(_isolate, this);
    auto value = v8::FunctionTemplate::New(_isolate, StaticOnPrepared, data)->GetFunction(context).ToLocalChecked();
    global->Set(context, V8_UTF_STRING(_isolate, "__onPrepared"), value);
}

int NodeRuntime::Start() {
    _threadId = std::this_thread::get_id();

    // Make argv memory adjacent
    char cmd[40];
    strcpy(cmd, "node -e __onPrepared();");
    int argc = 0;
    char *argv[32];
    char *p2 = strtok(cmd, " ");
    while (p2 && argc < 32 - 1) {
        argv[argc++] = p2;
        p2 = strtok(0, " ");
    }
    argv[argc] = 0;

    std::vector<std::string> args(argv, argv + argc);
    std::vector<std::string> exec_args;
    std::vector<std::string> errors;
    // Parse Node.js CLI options, and print any errors that have occurred while
    // trying to parse them.
    int exit_code = node::InitializeNodeWithArgs(&args, &exec_args, &errors);
    for (const std::string& error : errors)
        fprintf(stderr, "%s: %s\n", args[0].c_str(), error.c_str());
    if (exit_code != 0) {
        return exit_code;
    }

    // Create a v8::Platform instance. `MultiIsolatePlatform::Create()` is a way
    // to create a v8::Platform instance that Node.js can use when creating
    // Worker threads. When no `MultiIsolatePlatform` instance is present,
    // Worker threads are disabled.
    std::unique_ptr<node::MultiIsolatePlatform> platform =
            node::MultiIsolatePlatform::Create(4);
    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();

    // See below for the contents of this function.
    int ret = RunNodeInstance(platform.get(), args, exec_args);

    v8::V8::Dispose();
    v8::V8::ShutdownPlatform();
    return ret;
}

int NodeRuntime::RunNodeInstance(node::MultiIsolatePlatform* platform,
                                 const std::vector<std::string>& args,
                                 const std::vector<std::string>& exec_args) {
    int exit_code = 0;
    // Set up a libuv event loop.
    uv_loop_t* loop = uv_loop_new();
    int ret = uv_loop_init(loop);
    if (ret != 0) {
        fprintf(stderr, "%s: Failed to initialize loop: %s\n",
                args[0].c_str(),
                uv_err_name(ret));
        return 1;
    }

    std::shared_ptr<node::ArrayBufferAllocator> allocator =
            node::ArrayBufferAllocator::Create();

    v8::Isolate *isolate = node::NewIsolate(allocator, loop, platform);
    if (isolate == nullptr) {
        fprintf(stderr, "%s: Failed to initialize V8 Isolate\n", args[0].c_str());
        return 1;
    }

    {
        v8::Locker locker(isolate);
        v8::Isolate::Scope isolate_scope(isolate);

        // Create a node::IsolateData instance that will later be released using
        // node::FreeIsolateData().
        std::unique_ptr<node::IsolateData, decltype(&node::FreeIsolateData)> isolate_data(
                node::CreateIsolateData(isolate, loop, platform, allocator.get()),
                node::FreeIsolateData);

        // Set up a new v8::Context.
        v8::HandleScope handle_scope(isolate);
        v8::Local<v8::Context> context = node::NewContext(isolate);
        if (context.IsEmpty()) {
            fprintf(stderr, "%s: Failed to initialize V8 Context\n", args[0].c_str());
            return 1;
        }
        _eventLoop = loop;
        _isolate = isolate;
        _context.Reset(isolate, context);
        SetUp();

        // The v8::Context needs to be entered when node::CreateEnvironment() and
        // node::LoadEnvironment() are being called.
        v8::Context::Scope context_scope(context);

        // Create a node::Environment instance that will later be released using
        // node::FreeEnvironment().
        std::unique_ptr<node::Environment, decltype(&node::FreeEnvironment)> env(
                node::CreateEnvironment(isolate_data.get(), context, args, exec_args),
                node::FreeEnvironment);

        // Set up the Node.js instance for execution, and run code inside of it.
        // There is also a variant that takes a callback and provides it with
        // the `require` and `process` objects, so that it can manually compile
        // and run scripts as needed.
        // The `require` function inside this script does *not* access the file
        // system, and can only load built-in Node.js modules.
        // `module.createRequire()` is being used to create one that is able to
        // load files from the disk, and uses the standard CommonJS file loader
        // instead of the internal-only `require` function.
        // v8::MaybeLocal<v8::Value> loadenv_ret = node::LoadEnvironment(
        //         env.get(),
        //         "const publicRequire ="
        //         "  require('module').createRequire(process.cwd() + '/');"
        //         "globalThis.require = publicRequire;"
        //         "require('vm').runInThisContext(process.argv[1]);");
        v8::MaybeLocal<v8::Value> loadenv_ret
                = node::LoadEnvironment(env.get(),
                                        "const publicRequire ="
                                        "  require('module').createRequire(process.cwd() + '/');"
                                        "globalThis.require = publicRequire;"
                                        "require('vm').runInThisContext(process.argv[1]);"
                                        "__onPrepared();");
        if (loadenv_ret.IsEmpty())  // There has been a JS exception.
            return 1;

        {
            // SealHandleScope protects against handle leaks from callbacks.
            v8::SealHandleScope seal(isolate);
            int more;
            do {
                uv_run(loop, UV_RUN_DEFAULT);

                // V8 tasks on background threads may end up scheduling new tasks in the
                // foreground, which in turn can keep the event loop going. For example,
                // WebAssembly.compile() may do so.
                platform->DrainTasks(isolate);

                // If there are new tasks, continue.
                more = uv_loop_alive(loop);
                if (more) continue;

                // node::EmitBeforeExit() is used to emit the 'beforeExit' event on
                // the `process` object.
                node::EmitBeforeExit(env.get());

                // 'beforeExit' can also schedule new work that keeps the event loop
                // running.
                more = uv_loop_alive(loop);
            } while (more);
        }

        // node::EmitExit() returns the current exit code.
        exit_code = node::EmitExit(env.get());

        // node::Stop() can be used to explicitly stop the event loop and keep
        // further JavaScript from running. It can be called from any thread,
        // and will act like worker.terminate() if called from another thread.
        node::Stop(env.get());
    }

    // Unregister the Isolate with the platform and add a listener that is called
    // when the Platform is done cleaning up any state it had associated with
    // the Isolate.
    bool platform_finished = false;
    platform->AddIsolateFinishedCallback(isolate, [](void* data) {
        *static_cast<bool*>(data) = true;
    }, &platform_finished);
    platform->UnregisterIsolate(isolate);
    isolate->Dispose();

    // Wait until the platform has cleaned up all relevant resources.
    while (!platform_finished)
        uv_run(loop, UV_RUN_ONCE);
    int err = uv_loop_close(loop);
    uv_loop_delete(loop);
    assert(err == 0);

    return exit_code;
}

void NodeRuntime::Dispose() {
    // v8_platform.StopTracingAgent();
    // v8_initialized = false;
    // V8::Dispose();

    // uv_run cannot be called from the time before the beforeExit callback
    // runs until the program exits unless the event loop has any referenced
    // handles after beforeExit terminates. This prevents unrefed timers
    // that happen to terminate during shutdown from being run unsafely.
    // Since uv_run cannot be called, uv_async handles held by the platform
    // will never be fully cleaned up.
    // v8_platform.Dispose();
}

NodeRuntime *NodeRuntime::GetCurrent(const v8::FunctionCallbackInfo<v8::Value> &info) {
    assert(info.Data()->IsExternal());
    auto external = info.Data().As<v8::External>();
    return reinterpret_cast<NodeRuntime *>(external->Value());
}

jobject NodeRuntime::Wrap(JNIEnv *env, v8::Persistent<v8::Value> *value, JSType type) {
    switch (type) {
        case Null:
            return this->jNull;
        case Undefined:
            return this->jUndefined;
        case Boolean: {
            v8::Local<v8::Value> local = value->Get(_isolate);
            v8::Local<v8::Boolean> target = local->ToBoolean(_isolate);
            if (target->Value()) {
                return this->jTrue;
            } else {
                return this->jFalse;
            }
        }
        case Object:
            return JSObject::Wrap(env, this, value);
        case String:
            return JSString::Wrap(env, this, value);
        case Number:
            return JSNumber::Wrap(env, this, value);
        case Function:
            return JSFunction::Wrap(env, this, value);
        case Promise:
            return JSPromise::Wrap(env, this, value);
        case Array:
            return JSArray::Wrap(env, this, value);
        case Error:
            return JSError::Wrap(env, this, value);
        default:
            return JSValue::Wrap(env, this, value);
    }
}

void NodeRuntime::TryLoop() {
    if (!_eventLoop) {
        LOGE("TryLoop but eventLoop is NULL");
        return;
    }
    if (!asyncHandle) {
        asyncHandle = new uv_async_t();
        asyncHandle->data = this;
        uv_async_init(_eventLoop, asyncHandle, NodeRuntime::StaticHandle);
        uv_async_send(asyncHandle);
    }
}

bool NodeRuntime::Await(std::function<void()> runnable) {
    assert(_isolate != nullptr);
    if (std::this_thread::get_id() == _threadId) {
        runnable();
        return true;
    } else {
#ifdef NODE_DEBUG
        ENTER_JNI(_vm)
            env->ThrowNew(env->FindClass("java/lang/IllegalStateException"), "Illegal thread access");
        EXIT_JNI(_vm)
        return true;
#else
        std::condition_variable cv;
        bool signaled = false;
        auto callback = [&]() {
            runnable();
            {
                std::lock_guard<std::mutex> lock(asyncMutex);
                signaled = true;
            }
            cv.notify_one();
        };

        std::lock_guard<std::mutex> instanceLock(instanceMutex);
        std::unique_lock<std::mutex> lock(asyncMutex);
        if (!running) {
            LOGE("Instance has been destroyed, ignore await");
            return false;
        }
        callbacks.push_back(callback);
        this->TryLoop();

        cv.wait(lock, [&] { return signaled; });
        lock.unlock();
        return true;
#endif
    }
}

bool NodeRuntime::Post(std::function<void()> runnable) {
    if (std::this_thread::get_id() == _threadId) {
        runnable();
        return true;
    } else {
        std::lock_guard<std::mutex> lock(asyncMutex);
        if (!_running) {
            runnable();
            return false;
        }
        callbacks.push_back(runnable);
        this->TryLoop();
        return true;
    }
}

void NodeRuntime::Handle(uv_async_t *handle) {
    asyncMutex.lock();
    while (!callbacks.empty()) {
        auto callback = callbacks.front();
        asyncMutex.unlock();
        callback();
        asyncMutex.lock();
        callbacks.erase(callbacks.begin());
    }
    uv_close((uv_handle_t *) handle, [](uv_handle_t *h) {
        delete (uv_async_t *) h;
    });
    asyncHandle = nullptr;
    asyncMutex.unlock();
}

JSType NodeRuntime::GetType(v8::Local<v8::Value> &value) {
    if (value->IsNull()) {
        return JSType::Null;
    } else if (value->IsUndefined()) {
        return JSType::Undefined;
    } else if (value->IsBoolean()) {
        return JSType::Boolean;
    } else if (value->IsNumber()) {
        return JSType::Number;
    } else if (value->IsObject()) {
        if (value->IsFunction()) {
            return JSType::Function;
        } else if (value->IsPromise()) {
            return JSType::Promise;
        } else if (value->IsNativeError()) {
            return JSType::Error;
        } else if (value->IsArray()) {
            return JSType::Array;
        }
        return JSType::Object;
    } else if (value->IsString()) {
        return JSType::String;
    }
    return JSType::Value;
}

v8::Local<v8::Object> NodeRuntime::Require(const char *path) {
    v8::EscapableHandleScope handleScope(_isolate);
    auto context = _context.Get(_isolate);
    auto global = _global->Get(_isolate);
    v8::Context::Scope contextScope(context);
    auto key = v8::String::NewFromUtf8(_isolate, "require").ToLocalChecked();
    v8::Local<v8::Object> require = global->Get(context, key).ToLocalChecked()->ToObject(context).ToLocalChecked();
    v8::Local<v8::Value> *argv = new v8::Local<v8::Value>[1];
    argv[0] = V8_UTF_STRING(_isolate, path);
    assert(require->IsFunction());
    auto result = require->CallAsFunction(context, global, 1, argv).ToLocalChecked();
    return handleScope.Escape(result->ToObject(context).ToLocalChecked());
}
