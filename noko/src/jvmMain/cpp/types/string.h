#ifndef NODE_JSSTRING_H
#define NODE_JSSTRING_H

#include <jni.h>
#include "v8.h"

namespace String {

jobject Of(JNIEnv *env, v8::Local<v8::Value> &value);

v8::Local<v8::String> Value(JNIEnv *env, jstring value);

jint OnLoad(JNIEnv *env);

bool Is(JNIEnv *env, jobject obj);
}

#endif //NODE_JSSTRING_H
