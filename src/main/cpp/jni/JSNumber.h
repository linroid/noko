//
// Created by linroid on 2019-10-21.
//

#ifndef NODE_JSNUMBER_H
#define NODE_JSNUMBER_H

#include <jni.h>
#include "JSContext.h"

class JSNumber {
public:
    static jobject Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Local<v8::Number> &value);

    JNICALL static void New(JNIEnv *env, jobject jthis, jdouble jdata);

    static jint OnLoad(JNIEnv *env);
};

#endif //NODE_JSNUMBER_H
