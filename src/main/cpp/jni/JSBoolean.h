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
    inline static jobject Wrap(JNIEnv *env, NodeRuntime *runtime, bool value) {
        return env->NewObject(jClazz, jConstructor, runtime->jContext, value);
    }

    static jint OnLoad(JNIEnv *env);
};

#endif //NODE_JSBOOLEAN_H
