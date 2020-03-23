//
// Created by linroid on 2019-10-21.
//

#ifndef NODE_JSNUMBER_H
#define NODE_JSNUMBER_H

#include <jni.h>
#include "JSContext.h"

class JSNumber {
private:
    static jclass jClazz;
    static jmethodID jConstructor;

public:
    inline static jobject Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Persistent<v8::Value> *value) {
        return env->NewObject(jClazz, jConstructor, runtime->jContext_, (jlong) value);
    }

    JNICALL static void New(JNIEnv *env, jobject jThis, jdouble jData);

    static jint OnLoad(JNIEnv *env);
};

#endif //NODE_JSNUMBER_H
