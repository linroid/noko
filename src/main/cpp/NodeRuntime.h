//
// Created by linroid on 2019-10-19.
//

#ifndef NODE_NODE_RUNTIME_H
#define NODE_NODE_RUNTIME_H

#include <jni.h>
#include <string>
#include <vector>
#include <thread>
#include <node.h>
#include <uv.h>
#include "macros.h"

enum JSType {
    None,
    Undefined,
    Null,
    Value,
    Array,
    Object,
    String,
    Number,
    Boolean,
    Function,
    Promise,
    Error,
};

class NodeRuntime {

private:
    jobject jThis = nullptr;
    jmethodID onBeforeStart = nullptr;
    jmethodID onBeforeExit = nullptr;

    bool _running = false;
    std::thread::id _threadId;
    std::mutex asyncMutex;
    std::mutex instanceMutex;
    std::vector<std::function<void()>> callbacks;
    uv_async_t *asyncHandle = nullptr;
    int id = -1;

    static std::mutex sharedMutex;
    static jint instanceCount;
    static jint seq;

    static void StaticOnPrepared(const v8::FunctionCallbackInfo<v8::Value> &info);

    static void StaticBeforeExit(const v8::FunctionCallbackInfo<v8::Value> &info);

    static void StaticHandle(uv_async_t *handle);

    void Handle(uv_async_t *handle);

    void TryLoop();

    int RunNodeInstance(node::MultiIsolatePlatform *platform,
                                     const std::vector<std::string> &args,
                                     const std::vector<std::string> &exec_args);
    void SetUp();

public:
    jobject jContext = nullptr;
    jobject jNull = nullptr;
    jobject jUndefined = nullptr;
    jobject jTrue = nullptr;
    jobject jFalse = nullptr;
    JavaVM *_vm = nullptr;

    v8::Isolate *_isolate = nullptr;
    v8::Persistent<v8::Context> _context;
    v8::Persistent<v8::Object> *_global = nullptr;
    uv_loop_t *_eventLoop;

    NodeRuntime(JNIEnv *env, jobject jThis, jmethodID onBeforeStart, jmethodID onBeforeExit);

    ~NodeRuntime();

    int Start();

    void Dispose();

    void OnPrepared();

    bool Await(std::function<void()> runnable);

    bool Post(std::function<void()> runnable);

    jobject Wrap(JNIEnv *env, v8::Persistent<v8::Value> *value, JSType type);

    JSType GetType(v8::Local<v8::Value> &value);

    v8::Local<v8::Object> Require(const char *path);

    static inline NodeRuntime *GetCurrent(const v8::FunctionCallbackInfo<v8::Value> &info);
};

#endif //NODE_NODE_RUNTIME_H
