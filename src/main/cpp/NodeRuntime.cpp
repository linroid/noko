//
// Created by linroid on 2019-10-19.
//

#include <jni.h>
#include <mutex>
#include <uv.h>
#include "node.h"
#include "v8.h"
#include "util.h"
#include "util.h"
#include "NodeRuntime.h"

void beforeStartCallback(const v8::FunctionCallbackInfo<v8::Value> &info) {
    NodeRuntime *runtime = NodeRuntime::GetCurrent(info);
    runtime->beforeStart();
}

void onExit(const v8::FunctionCallbackInfo<v8::Value> &args) {
    auto *env = node::Environment::GetCurrent(args);
    int code = args[0]->Int32Value(env->context()).FromMaybe(0);
    LOGI("onExit: %d", code);
    uv_walk(env->event_loop(), [](uv_handle_t *h, void *arg) {
        uv_unref(h);
    }, nullptr);
    uv_stop(env->event_loop());
}

void NodeRuntime::beforeStart() {
    LOGI("beforeStart");
    JNIEnv *env;
    auto stat = vm->GetEnv((void **) (&env), JNI_VERSION_1_6);
    if (stat == JNI_EDETACHED) {
        vm->AttachCurrentThread(&env, nullptr);
    }
    auto javaContext = JSContext::NewJava(env, runtime);
    if (javaContext == nullptr) {
        throwError(env, "Failed to new JSContext instance");
    }
    auto isolate = runtime->isolate;
    auto global = runtime->global->Get(isolate);
    global->Set(v8::String::NewFromUtf8(isolate, "__beforeStart"), v8::Undefined(isolate));
    env->CallVoidMethod(thiz, onBeforeStart, javaContext);
    if (stat == JNI_EDETACHED) {
        vm->DetachCurrentThread();
    }
}

void NodeRuntime::onEnvReady(node::Environment *nodeEnv) {
    LOGI("onReady");
    v8::HandleScope handleScope(nodeEnv->isolate());
    v8::Context::Scope contextScope(nodeEnv->context());
    auto isolate = nodeEnv->isolate();
    auto context = nodeEnv->context();
    auto process = nodeEnv->process_object();
    nodeEnv->SetMethod(process, "reallyExit", onExit);
    nodeEnv->SetMethod(process, "abort", onExit);
    nodeEnv->SetMethod(process, "_kill", onExit);
    auto runtime = new V8Runtime();
    runtime->nodeEnv = nodeEnv;
    runtime->running = true;
    runtime->isolateData = nodeEnv->isolate_data();
    runtime->isolate = nodeEnv->isolate();
    runtime->context.Reset(isolate, context);

    auto global = context->Global();
    runtime->global = new v8::Persistent<v8::Object>(isolate, global);
    runtime->locker = new v8::Locker(isolate);

    JNIEnv *env;
    auto stat = vm->GetEnv((void **) (&env), JNI_VERSION_1_6);
    if (stat == JNI_EDETACHED) {
        vm->AttachCurrentThread(&env, nullptr);
    }
    auto javaContext = JSContext::NewJava(env, runtime);
    if (javaContext == nullptr) {
        throwError(env, "Failed to new JSContext instance");
    }
    auto data = v8::External::New(isolate, runtime);
    global->Set(v8::String::NewFromUtf8(isolate, "__beforeStart",
                                        v8::NewStringType::kNormal).ToLocalChecked(),
                v8::FunctionTemplate::New(isolate, beforeStartCallback, data)->GetFunction());
    runtime->javaContext = env->NewGlobalRef(javaContext);
    // env->CallVoidMethod(thiz, onContextReady, javaContext);

    if (stat == JNI_EDETACHED) {
        vm->DetachCurrentThread();
    }
}

int NodeRuntime::start(std::vector<std::string> &args) {
    int argc = args.size();
    int len = argc;
    for (auto &arg : args) {
        len += arg.size();
    }
    char *data = new char[len];
    int offset = 0;
    // Make argv memory adjacent
    char **argv = new char *[argc];
    for (auto i = 0; i < argc; ++i) {
        argv[i] = data + offset;
        strcpy(data + offset, args[i].c_str());
        offset += args[i].size();
        data[offset + 1] = '\0';
        ++offset;
    }

    int ret = node::Start(argc, argv, [this](void *env) {
        onEnvReady(static_cast<node::Environment *>(env));
    });
    delete[] data;
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

NodeRuntime::NodeRuntime(JNIEnv *env, jobject thiz, jmethodID onBeforeStart, jmethodID onBeforeExit) {
    env->GetJavaVM(&vm);
    this->onBeforeStart = onBeforeStart;
    this->onBeforeExit = onBeforeExit;
    this->thiz = env->NewGlobalRef(thiz);
}

NodeRuntime::~NodeRuntime() {
    delete runtime;
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
