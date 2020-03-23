//
// Created by linroid on 2019-10-21.
//

#ifndef NODE_JSBOOLEAN_H
#define NODE_JSBOOLEAN_H

#include <jni.h>
#include "../NodeRuntime.h"
#include "JSValue.h"

class JSBoolean {
private:
    static jclass jClazz;
    static jmethodID jConstructor;

public:
    inline static jobject Wrap(JNIEnv *env, NodeRuntime *runtime, v8::Persistent<v8::Value> *value, bool data) {
        return env->NewObject(jClazz, jConstructor, runtime->jContext_, (jlong) value, data);
    }

    static jint OnLoad(JNIEnv *env);
};

#endif //NODE_JSBOOLEAN_H
