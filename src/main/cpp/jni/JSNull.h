//
// Created by linroid on 2019-10-23.
//

#ifndef NODE_JSNULL_H
#define NODE_JSNULL_H

#include <jni.h>
#include "../NodeRuntime.h"

class JSNull {
public:
    static jobject Wrap(JNIEnv *env, NodeRuntime *runtime);

    JNICALL static void New(JNIEnv *env, jobject jthis);

    static jint OnLoad(JNIEnv *env);
};


#endif //NODE_JSNULL_H
