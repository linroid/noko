#ifndef NODE_JSVALUE_H
#define NODE_JSVALUE_H

#include <jni.h>
#include "../runtime.h"
#include "../util/jni_helper.h"
#include "integer.h"
#include "long.h"
#include "double.h"
#include "string.h"
#include "boolean.h"

class JsValue {
 private:
  static jfieldID pointer_field_id_;
  static jmethodID init_method_id_;
  static jmethodID get_node_pointer_id_;
 public:
  static jclass class_;

  inline static jlong GetPointer(JNIEnv *env, jobject obj) {
    return env->GetLongField(obj, pointer_field_id_);
  }

  inline static void SetPointer(JNIEnv *env, jobject obj, jlong value) {
    env->SetLongField(obj, pointer_field_id_, value);
  }

  inline static jobject Of(JNIEnv *env, jobject node, jlong pointer) {
    return env->NewObject(class_, init_method_id_, node, pointer);
  }

  inline static bool Is(JNIEnv *env, jobject obj) {
    return env->IsInstanceOf(obj, class_);
  }

  inline static v8::Persistent<v8::Value> *Unwrap(JNIEnv *env, jobject obj) {
    return reinterpret_cast<v8::Persistent<v8::Value> *>(GetPointer(env, obj));
  }

  inline static Runtime *GetRuntime(JNIEnv *env, jobject obj) {
    auto pointer = env->CallLongMethod(obj, get_node_pointer_id_);
    if (pointer == 0) return nullptr;
    return reinterpret_cast<Runtime *>(pointer);
  }

  JNICALL static jstring ToString(JNIEnv *env, jobject j_this);

  JNICALL static jstring TypeOf(JNIEnv *env, jobject j_this);

  JNICALL static jstring ToJson(JNIEnv *env, jobject j_this);

  JNICALL static jdouble ToNumber(JNIEnv *env, jobject j_this);

  JNICALL static jboolean Equals(JNIEnv *env, jobject j_this, jobject j_other);

  JNICALL static void Dispose(JNIEnv *env, jobject j_this);

  static inline v8::Local<v8::Value> Value(v8::Local<v8::Context> context,
                                    v8::Isolate *isolate,
                                    JNIEnv *env,
                                    jobject obj) {
    if (obj == nullptr) {
      return v8::Null(isolate);
    } else if (JsValue::Is(env, obj)) {
      return JsValue::Unwrap(env, obj)->Get(isolate);
    } else if (String::Is(env, obj)) {
      return String::Value(env, (jstring) obj);
    } else if (Boolean::Is(env, obj)) {
      return Boolean::Value(env, obj) ? v8::True(isolate) : v8::False(isolate);
    } else if (Integer::Is(env, obj)) {
      return v8::Int32::New(isolate, Integer::Value(env, obj));
    } else if (Long::Is(env, obj)) {
      return v8::BigInt::New(isolate, Long::Value(env, obj));
    } else if (Double::Is(env, obj)) {
      return v8::Number::New(isolate, Double::Value(env, obj));
    } else {
      auto class_name = JniHelper::GetClassName(env, obj);
      LOGE("Not supported type: %s", class_name.c_str());
      abort();
    }
  }

  static jint OnLoad(JNIEnv *env);
};

#endif //NODE_JSVALUE_H
