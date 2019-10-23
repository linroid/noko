//
// Created by linroid on 2019-10-21.
//

#ifndef NODE_JSNUMBER_H
#define NODE_JSNUMBER_H

#include <jni.h>
#include "JSContext.h"

class JSNumber {
public:
    static jobject New(JNIEnv *env, V8Runtime *runtime);

    static jint OnLoad(JNIEnv *env);
};

#endif //NODE_JSNUMBER_H
