//
// Created by linroid on 2019-10-20.
//

#ifndef NODE_JSUNDEFINED_H
#define NODE_JSUNDEFINED_H

#include <jni.h>
#include "JSContext.h"

class JSUndefined {
private:
    static jclass jclazz;
    static jmethodID jconstructor;

public:
    inline static jobject Wrap(JNIEnv *env, NodeRuntime *runtime) {
        return env->NewObject(jclazz, jconstructor, runtime->jcontext);
    }

    static jint OnLoad(JNIEnv *env);
};

#endif //NODE_JSUNDEFINED_H
