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
    JNICALL static jint Size(JNIEnv *env, jobject thiz);

    static jobject Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Local<v8::Array> &value);

    static jint OnLoad(JNIEnv *env);
};


#endif //NODE_JSARRAY_H
