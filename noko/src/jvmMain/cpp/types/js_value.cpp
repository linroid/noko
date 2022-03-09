#include <jni.h>
#include "../runtime.h"
#include "js_value.h"
#include "js_error.h"
#include "js_promise.h"
#include "js_array.h"
#include "js_object.h"
namespace JsValue {

jfieldID pointer_field_id_;
jmethodID init_method_id_;
jmethodID get_node_pointer_id_;
jclass class_;

v8::Persistent<v8::Value> *GetPointer(JNIEnv *env, jobject obj) {
  return reinterpret_cast<v8::Persistent<v8::Value> *>(env->GetLongField(obj, pointer_field_id_));
}

void SetPointer(JNIEnv *env, jobject obj, v8::Persistent<v8::Value> *value) {
  env->SetLongField(obj, pointer_field_id_, (jlong) value);
}

jobject Of(JNIEnv *env, v8::Local<v8::Value> value) {
  auto runtime = Runtime::Current();
  auto node = runtime->j_this_;
  auto isolate = runtime->isolate_;
  if (value->IsNullOrUndefined()) {
    return nullptr;
  } else if (value->IsBoolean()) {
    return Boolean::Of(value);
  } else if (value->IsString()) {
    return String::Of(env, value);
  } else if (value->IsInt32()) {
    return Integer::Of(env, value);
  } else if (value->IsBigInt()) {
    return Long::Of(env, value);
  } else if (value->IsNumber()) {
    return Double::Of(env, value);
  } else {
    auto pointer = new v8::Persistent<v8::Value>(isolate, value);
    if (value->IsObject()) {
      if (value->IsFunction()) {
        return JsFunction::Of(env, node, (jlong) pointer);
      } else if (value->IsPromise()) {
        return JsPromise::Of(env, node, (jlong) pointer);
      } else if (value->IsNativeError()) {
        return JsError::Of(env, node, (jlong) pointer);
      } else if (value->IsArray()) {
        return JsArray::Of(env, node, (jlong) pointer);
      }
      return JsObject::Of(env, node, (jlong) pointer);
    }
    return env->NewObject(class_, init_method_id_, node, (jlong) pointer);
  }
}

bool Is(JNIEnv *env, jobject obj) {
  return env->IsInstanceOf(obj, class_);
}

jstring ToString(JNIEnv *env, jobject j_this) {
  SETUP(env, j_this, v8::Value);
  v8::TryCatch try_catch(isolate);
  if (that.IsEmpty()) {
    return env->NewStringUTF("null");
  }
  auto str = that->ToString(context);
  if (str.IsEmpty()) {
    runtime->Throw(env, try_catch.Exception());
    return nullptr;
  }
  v8::String::Value unique_string(isolate, str.ToLocalChecked());
  return env->NewString(*unique_string, unique_string.length());
}

jstring TypeOf(JNIEnv *env, jobject j_this) {
  SETUP(env, j_this, v8::Value);
  auto type = that->TypeOf(isolate);
  v8::String::Value unicode_string(isolate, type);
  uint16_t *unicode_chars = *unicode_string;
  return env->NewString(unicode_chars, unicode_string.length());
}

jstring ToJson(JNIEnv *env, jobject j_this) {
  SETUP(env, j_this, v8::Value);
  v8::TryCatch try_catch(isolate);
  auto str = v8::JSON::Stringify(context, that);
  if (str.IsEmpty()) {
    if (try_catch.HasCaught()) {
      runtime->Throw(env, try_catch.Exception());
      return nullptr;
    }
    return env->NewString(new uint16_t[0], 0);
  }
  auto value = str.ToLocalChecked();
  v8::String::Value unicode_string(isolate, value);
  uint16_t *unicode_chars = *unicode_string;
  return env->NewString(unicode_chars, unicode_string.length());
}

jdouble ToNumber(JNIEnv *env, jobject j_this) {
  SETUP(env, j_this, v8::Value);
  v8::TryCatch try_catch(isolate);
  auto number = that->ToNumber(context);
  if (number.IsEmpty()) {
    runtime->Throw(env, try_catch.Exception());
    return 0.0;
  }
  v8::Local<v8::Number> checked = number.ToLocalChecked();
  return checked->Value();
}

void Dispose(JNIEnv *env, jobject j_this) {
  auto value = GetPointer(env, j_this);
  value->Reset();
  delete value;
  SetPointer(env, j_this, nullptr);
}

jboolean Equals(JNIEnv *env, jobject j_this, jobject j_other) {
  SETUP(env, j_this, v8::Value);
  auto other = GetPointer(env, j_other)->Get(isolate);
  return that->Equals(context, other).ToChecked();
}

jint OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/noko/types/JsValue");
  if (!clazz) {
    return JNI_ERR;
  }
  class_ = (jclass) env->NewGlobalRef(clazz);
  init_method_id_ = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/Node;J)V");
  pointer_field_id_ = env->GetFieldID(clazz, "pointer", "J");
  get_node_pointer_id_ = env->GetMethodID(clazz, "nodePointer", "()J");

  JNINativeMethod methods[] = {
      {"nativeToString", "()Ljava/lang/String;", (void *) ToString},
      {"nativeTypeOf", "()Ljava/lang/String;", (void *) TypeOf},
      {"nativeToJson", "()Ljava/lang/String;", (void *) ToJson},
      {"nativeDispose", "()V", (void *) Dispose},
      {"nativeToNumber", "()D", (void *) ToNumber},
      {"nativeEquals", "(Lcom/linroid/noko/types/JsValue;)Z", (void *) Equals},
  };

  int rc = env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
  if (rc != JNI_OK) {
    return rc;
  }
  return JNI_OK;
}

}