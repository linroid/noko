//
// Created by linroid on 2019-10-19.
//

#ifndef NODE_NODE_RUNTIME_H
#define NODE_NODE_RUNTIME_H

#include <jni.h>
#include <string>
#include <vector>
#include <thread>
#include "v8.h"
#include "env.h"
#include "util.h"
#include "env-inl.h"

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
    jobject jThis;
    jmethodID onBeforeStart;
    jmethodID onBeforeExit;

    bool running;
    std::thread::id threadId;
    std::mutex asyncMutex;
    std::vector<std::function<void()>> callbacks;
    uv_async_t *asyncHandle = nullptr;

    static std::mutex mutex;
    static jint instanceCount;

    static void StaticHandle(uv_async_t *handle);

    void Handle(uv_async_t *handle);

    void PostAndWait(std::function<void()> runnable);

public:
    jobject jContext;
    jobject jNull;
    jobject jUndefined;
    jobject jTrue;
    jobject jFalse;
    JavaVM *vm = nullptr;

    v8::Isolate *isolate;
    v8::Persistent<v8::Context> context;
    v8::Persistent<v8::Object> process;
    v8::Persistent<v8::Object> *global;
    v8::Locker *locker;
    node::Environment *nodeEnv;
    node::IsolateData *isolateData;
    uv_loop_t *eventLoop;

    NodeRuntime(JNIEnv *env, jobject jThis, jmethodID onBeforeStart, jmethodID onBeforeExit);

    ~NodeRuntime();

    int Start();

    void Dispose();

    void OnPrepared();

    void Run(std::function<void()> runnable);

    void OnEnvReady(node::Environment *nodeEnv);

    jobject Wrap(JNIEnv *env, v8::Persistent<v8::Value> *value, JSType type);

    JSType GetType(v8::Local<v8::Value> &value);

    static inline NodeRuntime *GetCurrent(const v8::FunctionCallbackInfo<v8::Value> &info);
};

#endif //NODE_NODE_RUNTIME_H
