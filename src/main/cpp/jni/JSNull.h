//
// Created by linroid on 2019-10-23.
//

#ifndef NODE_JSNULL_H
#define NODE_JSNULL_H

#include <jni.h>
#include "../NodeRuntime.h"

class JSNull {
private:
    static jclass jclazz;
    static jmethodID jconstructor;

public:
    inline static jobject Wrap(JNIEnv *env, NodeRuntime *runtime) {
        return env->NewObject(jclazz, jconstructor, runtime->jcontext);
    }

    JNICALL static void New(JNIEnv *env, jobject jthis);

    static jint OnLoad(JNIEnv *env);
};

#endif //NODE_JSNULL_H
