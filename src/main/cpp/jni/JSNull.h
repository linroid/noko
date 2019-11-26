//
// Created by linroid on 2019-10-23.
//

#ifndef NODE_JSNULL_H
#define NODE_JSNULL_H

#include <jni.h>
#include "../NodeRuntime.h"

class JSNull {
private:
    static jclass jClazz;
    static jmethodID jConstructor;

public:
    inline static jobject Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Persistent<v8::Value> *value) {
        return env->NewObject(jClazz, jConstructor, runtime->jContext, (jlong) value);
    }

    JNICALL static void New(JNIEnv *env, jobject jThis);

    static jint OnLoad(JNIEnv *env);
};

#endif //NODE_JSNULL_H
