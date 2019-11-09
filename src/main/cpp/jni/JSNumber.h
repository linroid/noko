//
// Created by linroid on 2019-10-21.
//

#ifndef NODE_JSNUMBER_H
#define NODE_JSNUMBER_H

#include <jni.h>
#include "JSContext.h"

class JSNumber {
private:
    static jclass jclazz;
    static jmethodID jconstructor;

public:
    inline static jobject Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Persistent<v8::Value> *value) {
        return env->NewObject(jclazz, jconstructor, runtime->jcontext, (jlong) value);
    }

    JNICALL static void New(JNIEnv *env, jobject jthis, jdouble jdata);

    static jint OnLoad(JNIEnv *env);
};

#endif //NODE_JSNUMBER_H
