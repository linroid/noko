//
// Created by linroid on 2019-10-21.
//

#ifndef NODE_JSBOOLEAN_H
#define NODE_JSBOOLEAN_H

#include <jni.h>
#include "../NodeRuntime.h"
#include "JSValue.h"

class JSBoolean {
public:
    static jobject Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Local<v8::Value> &value);

    JNICALL static void New(JNIEnv *env, jobject jthis, jboolean jdata);

    static jint OnLoad(JNIEnv *env);
};

#endif //NODE_JSBOOLEAN_H
