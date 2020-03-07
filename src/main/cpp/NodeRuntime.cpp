//
// Created by linroid on 2019-10-19.
//

#include <jni.h>
#include <mutex>
#include <uv.h>
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

void NodeRuntime::StaticOnPrepared(const v8::FunctionCallbackInfo<v8::Value> &info) {
    LOGD("StaticOnPrepared");
    NodeRuntime *instance = NodeRuntime::GetCurrent(info);
    instance->OnPrepared();
}

void NodeRuntime::StaticBeforeExit(const v8::FunctionCallbackInfo<v8::Value> &info) {
    auto env = node::Environment::GetCurrent(info);
    int code = info[0]->Int32Value(env->context()).FromMaybe(0);
    LOGD("StaticBeforeExit: %d", code);
    uv_walk(env->event_loop(), [](uv_handle_t *h, void *arg) {
        uv_unref(h);
    }, nullptr);
}

void NodeRuntime::StaticHandle(uv_async_t *handle) {
    NodeRuntime *runtime = reinterpret_cast<NodeRuntime *>(handle->data);
    runtime->Handle(handle);
}

NodeRuntime::NodeRuntime(JNIEnv *env, jobject jThis, jmethodID onBeforeStart, jmethodID onBeforeExit)
        : onBeforeStart(onBeforeStart), onBeforeExit(onBeforeExit) {
    LOGD("Construct NodeRuntime: %d", std::this_thread::get_id());

    env->GetJavaVM(&vm);
    this->jThis = env->NewGlobalRef(jThis);

    sharedMutex.lock();
    ++instanceCount;
    sharedMutex.unlock();
}

NodeRuntime::~NodeRuntime() {
    LOGE("~NodeRuntime(): %p, thread_id=%d", this, std::this_thread::get_id());
    std::lock_guard<std::mutex> lock(asyncMutex);

    ENTER_JNI(vm)
        env->DeleteGlobalRef(jThis);
    EXIT_JNI(vm)

    uv_stop(eventLoop);
    eventLoop = nullptr;
    global = nullptr;
    isolate = nullptr;
    running = false;
    sharedMutex.lock();
    --instanceCount;
    sharedMutex.unlock();
    LOGE("~NodeRuntime() finished");
}

void NodeRuntime::OnPrepared() {
    LOGI("OnPrepared");
    auto localGlobal = global->Get(isolate);
    localGlobal->Delete(v8::String::NewFromUtf8(isolate, "__onPrepared"));

    ENTER_JNI(vm);
        env->CallVoidMethod(jThis, onBeforeStart, jContext);
    EXIT_JNI(vm);
}

void NodeRuntime::OnEnvReady(node::Environment *nodeEnv) {
    LOGI("OnEnvReady");
    v8::HandleScope handleScope(nodeEnv->isolate());
    v8::Context::Scope contextScope(nodeEnv->context());

    auto isolate = nodeEnv->isolate();
    auto context = nodeEnv->context();
    auto process = nodeEnv->process_object();
    auto global = context->Global();
    this->isolate = isolate;
    this->running = true;
    this->eventLoop = nodeEnv->event_loop();
    this->isolate = nodeEnv->isolate();
    this->context.Reset(isolate, context);
    this->process.Reset(isolate, process);
    this->global = new v8::Persistent<v8::Object>(isolate, global);

    ENTER_JNI(vm);
        auto nullValue = new v8::Persistent<v8::Value>(isolate, v8::Null(isolate));
        auto undefinedValue = new v8::Persistent<v8::Value>(isolate, v8::Undefined(isolate));
        auto trueValue = new v8::Persistent<v8::Value>(isolate, v8::True(isolate));
        auto falseValue = new v8::Persistent<v8::Value>(isolate, v8::False(isolate));
        this->jContext = env->NewGlobalRef(JSContext::Wrap(env, this));
        this->jNull = env->NewGlobalRef(JSNull::Wrap(env, this, nullValue));
        this->jUndefined = env->NewGlobalRef(JSUndefined::Wrap(env, this, undefinedValue));
        this->jTrue = env->NewGlobalRef(JSBoolean::Wrap(env, this, trueValue, true));
        this->jFalse = env->NewGlobalRef(JSBoolean::Wrap(env, this, falseValue, false));
        JSContext::SetShared(env, this);
    EXIT_JNI(vm);

    nodeEnv->SetMethod(process, "reallyExit", StaticBeforeExit);
    nodeEnv->SetMethod(process, "abort", StaticBeforeExit);
    nodeEnv->SetMethod(process, "_kill", StaticBeforeExit);

    auto data = v8::External::New(isolate, this);
    global->Set(V8_UTF_STRING(isolate, "__onPrepared"), v8::FunctionTemplate::New(isolate, StaticOnPrepared, data)->GetFunction());
}

int NodeRuntime::Start() {
    threadId = std::this_thread::get_id();

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

    int ret = node::Start(argc, argv, [this](void *env) {
        OnEnvReady(static_cast<node::Environment *>(env));
    });
    // delete[] data;
    // delete[] argv;
    return ret;
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
    CHECK(info.Data()->IsExternal());
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
            auto local = value->Get(isolate);
            v8::Local<v8::Boolean> target = local->ToBoolean(context.Get(isolate)).ToLocalChecked();
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
    if (!eventLoop) {
        LOGE("TryLoop but eventLoop is NULL");
        return;
    }
    if (!asyncHandle) {
        asyncHandle = new uv_async_t();
        asyncHandle->data = this;
        uv_async_init(eventLoop, asyncHandle, NodeRuntime::StaticHandle);
        uv_async_send(asyncHandle);
    }
}

void NodeRuntime::Await(std::function<void()> runnable) {
    CHECK_NOT_NULL(isolate);
    if (std::this_thread::get_id() == threadId) {
        runnable();
    } else {
        std::condition_variable cv;
        bool signaled = false;
        auto callback = [&]() {
            runnable();
            {
                std::lock_guard<std::mutex> lock(asyncMutex);
                LOGD("signaled = true;");
                signaled = true;
            }
            cv.notify_one();
        };

        std::unique_lock<std::mutex> lock(asyncMutex);
        if (!running) {
            LOGE("Instance has been destroyed, ignore await");
            return;
        }
        callbacks.push_back(callback);
        this->TryLoop();

        cv.wait(lock, [&] { return signaled && running; });
        LOGD("cv.wait(lock, [&] { return signaled; })");
        lock.unlock();
    }
}

void NodeRuntime::Async(std::function<void()> runnable) {
    if (std::this_thread::get_id() == threadId) {
        runnable();
    } else {
        std::lock_guard<std::mutex> lock(asyncMutex);
        callbacks.push_back(runnable);
        this->TryLoop();
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
