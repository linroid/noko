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
#include "V8Runtime.h"
#include "jni/JSContext.h"

class NodeRuntime {
private:
    jobject thiz;
    jmethodID onBeforeStart;
    jmethodID onBeforeExit;

    std::vector<std::string> args;
    V8Runtime *runtime = nullptr;
    JavaVM *vm = nullptr;
public:
    NodeRuntime(JNIEnv *env, jobject thiz, jmethodID onBeforeStart, jmethodID onBeforeExit);

    ~NodeRuntime();

    int start(std::vector<std::string> &args);

    void dispose();

    void beforeStart(const v8::FunctionCallbackInfo<v8::Value> &info);

    void onEnvReady(node::Environment *nodeEnv);
};

#endif //NODE_NODE_RUNTIME_H
