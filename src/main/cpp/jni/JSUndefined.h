//
// Created by linroid on 2019-10-20.
//

#ifndef NODE_JSUNDEFINED_H
#define NODE_JSUNDEFINED_H

#include <jni.h>
#include "JSContext.h"

class JSUndefined {
public:
    static jobject Wrap(JNIEnv *env, NodeRuntime *runtime);

    static jint OnLoad(JNIEnv *env);
};

#endif //NODE_JSUNDEFINED_H
