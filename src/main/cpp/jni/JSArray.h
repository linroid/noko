//
// Created by linroid on 2019-10-23.
//

#ifndef NODE_JSARRAY_H
#define NODE_JSARRAY_H

#include <jni.h>
#include "../NodeRuntime.h"
#include "macros.h"

class JSArray {
public:
    JNICALL static jint Size(JNIEnv *env, jobject jthis);

    JNICALL static void New(JNIEnv *env, jobject jthis);

    JNICALL static jobject Get(JNIEnv *env, jobject jthis, jint jindex);

    JNICALL static jboolean Add(JNIEnv *env, jobject jthis, jobject jelement);

    JNICALL static jboolean AddAt(JNIEnv *env, jobject jthis, jint jindex, jobject jelement);

    JNICALL static jboolean AddAll(JNIEnv *env, jobject jthis, jobjectArray jelements);

    JNICALL static jboolean AddAllAt(JNIEnv *env, jobject jthis, jint jindex, jobjectArray jelements);

    static jobject Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Persistent<v8::Value> *value);

    static jint OnLoad(JNIEnv *env);
};


#endif //NODE_JSARRAY_H
