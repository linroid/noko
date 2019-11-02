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
#include "jni/macros.h"
#include "jni/JSFunction.h"

void beforeStartCallback(const v8::FunctionCallbackInfo<v8::Value> &info) {
    NodeRuntime *instance = NodeRuntime::GetCurrent(info);
    instance->beforeStart();
}

void beforeExitCallback(const v8::FunctionCallbackInfo<v8::Value> &args) {
    auto *env = node::Environment::GetCurrent(args);
    int code = args[0]->Int32Value(env->context()).FromMaybe(0);
    LOGI("beforeExitCallback: %d", code);
    uv_walk(env->event_loop(), [](uv_handle_t *h, void *arg) {
        uv_unref(h);
    }, nullptr);
    uv_stop(env->event_loop());
}

void NodeRuntime::beforeStart() {
    LOGI("beforeStart");
    auto localGlobal = global->Get(isolate);
    localGlobal->Delete(v8::String::NewFromUtf8(isolate, "__beforeStart"));

    JNIEnv *env;
    auto stat = vm->GetEnv((void **) (&env), JNI_VERSION_1_6);
    if (stat == JNI_EDETACHED) {
        vm->AttachCurrentThread(&env, nullptr);
    }
    env->CallVoidMethod(thiz, onBeforeStart, javaContext);
    if (stat == JNI_EDETACHED) {
        vm->DetachCurrentThread();
    }
}

void NodeRuntime::onEnvReady(node::Environment *nodeEnv) {
    LOGI("onEnvReady");
    v8::HandleScope handleScope(nodeEnv->isolate());
    v8::Context::Scope contextScope(nodeEnv->context());

    auto isolate = nodeEnv->isolate();
    auto context = nodeEnv->context();
    auto process = nodeEnv->process_object();
    auto global = context->Global();

    nodeEnv->SetMethod(process, "reallyExit", beforeExitCallback);
    nodeEnv->SetMethod(process, "abort", beforeExitCallback);
    nodeEnv->SetMethod(process, "_kill", beforeExitCallback);

    auto data = v8::External::New(isolate, this);
    global->Set(V8_UTF_STRING("__beforeStart"), v8::FunctionTemplate::New(isolate, beforeStartCallback, data)->GetFunction());

    this->isolate = isolate;
    this->nodeEnv = nodeEnv;
    this->running = true;
    this->isolateData = nodeEnv->isolate_data();
    this->isolate = nodeEnv->isolate();
    this->context.Reset(isolate, context);
    this->process.Reset(isolate, process);
    this->global = new v8::Persistent<v8::Object>(isolate, global);
    this->locker = new v8::Locker(isolate);

    JNIEnv *env;
    auto stat = vm->GetEnv((void **) (&env), JNI_VERSION_1_6);
    if (stat == JNI_EDETACHED) {
        vm->AttachCurrentThread(&env, nullptr);
    }
    auto javaContext = JSContext::New(env, this);
    if (javaContext == nullptr) {
        throwError(env, "Failed to new JSContext instance");
    }
    this->javaContext = env->NewGlobalRef(javaContext);

    if (stat == JNI_EDETACHED) {
        vm->DetachCurrentThread();
    }
}

int NodeRuntime::start() {
    // Make argv memory adjacent
    char cmd[40];
    strcpy(cmd, "node -e global.__beforeStart();");
    int argc = 0;
    char *argv[32];
    char *p2 = strtok(cmd, " ");
    while (p2 && argc < 32 - 1) {
        argv[argc++] = p2;
        p2 = strtok(0, " ");
    }
    argv[argc] = 0;

    int ret = node::Start(argc, argv, [this](void *env) {
        onEnvReady(static_cast<node::Environment *>(env));
    });
    // delete[] data;
    // delete[] argv;
    return ret;
}

void NodeRuntime::dispose() {
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

NodeRuntime::NodeRuntime(JNIEnv *env, jobject thiz, jmethodID onBeforeStart, jmethodID onBeforeExit)
        : onBeforeStart(onBeforeStart), onBeforeExit(onBeforeExit) {
    env->GetJavaVM(&vm);
    this->thiz = env->NewGlobalRef(thiz);
}

NodeRuntime::~NodeRuntime() {
    JNIEnv *env;
    auto stat = vm->GetEnv((void **) (&env), JNI_VERSION_1_6);
    if (stat == JNI_EDETACHED) {
        vm->AttachCurrentThread(&env, nullptr);
    }
    env->DeleteGlobalRef(thiz);
    if (stat == JNI_EDETACHED) {
        vm->DetachCurrentThread();
    }
}

NodeRuntime *NodeRuntime::GetCurrent(const v8::FunctionCallbackInfo<v8::Value> &info) {
    CHECK(info.Data()->IsExternal());
    auto external = info.Data().As<v8::External>();
    return reinterpret_cast<NodeRuntime *>(external->Value());
}

jobject NodeRuntime::Wrap(JNIEnv *env, v8::Local<v8::Value> &value) {
    if (value->IsUndefined()) {
        return JSUndefined::New(env, this);
    } else if (value->IsBoolean()) {
        return JSBoolean::New(env, this, value);
    } else if (value->IsNumber()) {
        return JSNumber::New(env, this);
    } else if (value->IsObject()) {
        if (value->IsFunction()) {
            return JSFunction::New(env, this, value);
        }
        return JSObject::New(env, this, value);
    } else if (value->IsString()) {
        auto casted = value.As<v8::String>();
        return JSString::New(env, this, casted);
    }
    return JSValue::New(env, this, value);
}

void NodeRuntime::throwJSError(JNIEnv *env, v8::TryCatch &tryCatch) {
    LOGE("JSError");
    auto exception = tryCatch.Exception();
    v8::String::Utf8Value exceptionStr(exception);
    throwError(env, *exceptionStr);
}
