// #ifndef NODE_JS_CONTEXT_H
// #define NODE_JS_CONTEXT_H
//
// #include <jni.h>
// #include "../Node.h"
//
// class JSContext {
// private:
//   static jclass jClazz;
//
// public:
//   static void ClearPtr(JNIEnv *env, jobject obj) {
//     env->SetLongField(obj, jPtr, 0l);
//   }
//
//   // static JNICALL jobject ParseJson(JNIEnv *env, jobject jThis, jstring jJson);
//   //
//   // static JNICALL jobject Eval(JNIEnv *env, jobject jThis, jstring jCode, jstring jSource, jint jLine);
//   //
//   // static JNICALL jobject ThrowError(JNIEnv *env, jobject jThis, jstring jMessage);
//   //
//   // static JNICALL jobject Require(JNIEnv *env, jobject jThis, jstring jPath);
//
//   // static JNICALL void ClearReference(JNIEnv *env, jobject jThis, jlong ref);
//
//   static jint OnLoad(JNIEnv *env);
//
//   static void SetShared(JNIEnv *env, jobject jNode);
// };
//
//
// #endif //NODE_JS_CONTEXT_H
