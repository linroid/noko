//
// Created by linroid on 2019-10-19.
//

#ifndef NODE_NODE_RUNTIME_H
#define NODE_NODE_RUNTIME_H

#include <jni.h>
#include <string>
#include <vector>
#include "v8.h"
#include "env.h"
#include "util.h"

struct JNIClass {
    jclass clazz;
    jmethodID constructor;
    jfieldID reference;
    jfieldID context;
    jfieldID runtimePtr;
};

class NodeRuntime {
private:
    jobject thiz;
    jmethodID onBeforeStart;
    jmethodID onBeforeExit;
    JavaVM *vm = nullptr;

    bool running;

public:
    jobject javaContext;

    v8::Isolate *isolate;
    v8::Persistent<v8::Context> context;
    v8::Persistent<v8::Object> process;
    v8::Persistent<v8::Object> *global;
    v8::Locker *locker;
    node::Environment *nodeEnv;
    node::IsolateData *isolateData;
    uv_loop_t *uvLoop;

    NodeRuntime(JNIEnv *env, jobject thiz, jmethodID onBeforeStart, jmethodID onBeforeExit);

    ~NodeRuntime();

    int start(std::vector<std::string> &args);

    void dispose();

    void beforeStart();

    void onEnvReady(node::Environment *nodeEnv);

    jobject Wrap(JNIEnv *env, v8::Local<v8::Value> &value);

    static inline NodeRuntime *GetCurrent(const v8::FunctionCallbackInfo<v8::Value> &info);
};

#endif //NODE_NODE_RUNTIME_H
