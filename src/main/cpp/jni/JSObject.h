//
// Created by linroid on 2019-10-20.
//

#ifndef NODE_JSOBJECT_H
#define NODE_JSOBJECT_H

#include <jni.h>
#include "JSContext.h"
#include "../util.h"

class JSObject {

public:
    static jobject Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Local<v8::Value> &value);

    static jint OnLoad(JNIEnv *env);

    static JNICALL jobject Get(JNIEnv *env, jobject jthis, jstring key);

    static JNICALL void Set(JNIEnv *env, jobject jthis, jstring key, jobject);

    static JNICALL void New(JNIEnv *env, jobject jthis);
};

#endif //NODE_JSOBJECT_H
