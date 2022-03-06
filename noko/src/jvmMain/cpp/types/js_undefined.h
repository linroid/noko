#ifndef NODE_JSUNDEFINED_H
#define NODE_JSUNDEFINED_H

#include <jni.h>
#include <v8.h>

namespace JsUndefined {

jobject Of(JNIEnv *env, jobject node, jlong pointer);

jint OnLoad(JNIEnv *env);

}

#endif //NODE_JSUNDEFINED_H
