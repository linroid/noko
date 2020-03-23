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
    kNone,
    kUndefined,
    kNull,
    kValue,
    kArray,
    kObject,
    kString,
    kNumber,
    kBoolean,
    kFunction,
    kPromise,
    kError,
};

class NodeRuntime {

private:
    jobject jThis_ = nullptr;
    jmethodID onBeforeStart_ = nullptr;
    jmethodID onBeforeExit_ = nullptr;

    bool running_ = false;
    std::thread::id threadId_;
    std::mutex asyncMutex_;
    std::mutex instanceMutex_;
    std::vector<std::function<void()>> callbacks_;
    uv_async_t *asyncHandle_ = nullptr;
    int id_ = -1;

    static std::mutex sharedMutex_;
    static jint instanceCount_;
    static jint seq_;
    uv_loop_t *eventLoop_;

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
    jobject jContext_ = nullptr;
    jobject jNull_ = nullptr;
    jobject jUndefined_ = nullptr;
    jobject jTrue_ = nullptr;
    jobject jFalse_ = nullptr;
    JavaVM *vm_ = nullptr;

    v8::Isolate *isolate_ = nullptr;
    v8::Persistent<v8::Context> context_;
    v8::Persistent<v8::Object> *global_ = nullptr;

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
