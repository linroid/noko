#include <string>
#include "string.h" // NOLINT(modernize-deprecated-headers)
#include "js_value.h"

namespace String {

jclass class_;

jobject Of(JNIEnv *env, v8::Local<v8::Value> &value) {
  auto isolate = Runtime::Current()->isolate_;
  v8::String::Value unicode_string(isolate, value);
  return env->NewString(*unicode_string, unicode_string.length());
}

v8::Local<v8::String> Value(JNIEnv *env, jstring value) {
  const uint16_t *content = env->GetStringChars(value, nullptr);
  const jint length = env->GetStringLength(value);
  auto isolate = Runtime::Current()->isolate_;
  v8::EscapableHandleScope handle_scope(isolate);
  auto result = v8::String::NewFromTwoByte(isolate, content, v8::NewStringType::kNormal, length);
  env->ReleaseStringChars(value, content);
  return handle_scope.Escape(result.ToLocalChecked());
}

bool Is(JNIEnv *env, jobject obj) {
  return env->IsInstanceOf(obj, class_);
}

jint OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("java/lang/String");
  if (!clazz) {
    return JNI_ERR;
  }
  class_ = (jclass) env->NewGlobalRef(clazz);
  return JNI_OK;
}

}
