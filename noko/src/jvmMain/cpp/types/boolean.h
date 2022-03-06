#ifndef NODE_JSBOOLEAN_H
#define NODE_JSBOOLEAN_H

#include <jni.h>
#include "js_value.h"
namespace Boolean {

jobject Of(v8::Local<v8::Value> &value);

inline jboolean Value(JNIEnv *env, jobject obj);

bool Is(JNIEnv *env, jobject obj);

jint OnLoad(JNIEnv *env);

}

#endif //NODE_JSBOOLEAN_H
