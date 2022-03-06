#ifndef NODE_JSARRAY_H
#define NODE_JSARRAY_H

#include <jni.h>

namespace JsArray {

jobject Of(JNIEnv *env, jobject node, jlong pointer);

jint OnLoad(JNIEnv *env);

}

#endif //NODE_JSARRAY_H
