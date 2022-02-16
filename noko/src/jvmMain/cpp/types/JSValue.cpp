#include <jni.h>
#include "../Noko.h"
#include "JSValue.h"
#include "JSError.h"

jmethodID JSValue::jConstructor;
jclass JSValue::jClazz;
jfieldID JSValue::jReference;
jmethodID JSValue::jGetNoko;

jint JSValue::OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/noko/types/JSValue");
  if (!clazz) {
    return JNI_ERR;
  }
  jClazz = (jclass) env->NewGlobalRef(clazz);
  jConstructor = env->GetMethodID(clazz, "<init>", "(Lcom/linroid/noko/types/JSContext;J)V");
  jReference = env->GetFieldID(clazz, "reference", "J");
  jGetNoko = env->GetMethodID(clazz, "nokoPtr", "()J");

  JNINativeMethod methods[] = {
      {"nativeToString", "()Ljava/lang/String;", (void *) JSValue::ToString},
      {"nativeTypeOf",   "()Ljava/lang/String;", (void *) JSValue::TypeOf},
      {"nativeToJson",   "()Ljava/lang/String;", (void *) JSValue::ToJson},
      {"nativeDispose",  "()V",                  (void *) JSValue::Dispose},
      {"nativeToNumber", "()D",                  (void *) JSValue::ToNumber},
  };

  int rc = env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
  if (rc != JNI_OK) {
    return rc;
  }
  return JNI_OK;
}

jstring JSValue::ToString(JNIEnv *env, jobject jThis) {
  SETUP(env, jThis, v8::Value)
  v8::TryCatch tryCatch(isolate);
  if (that.IsEmpty()) {
    LOGE("Call toString on null object!!!");
    return env->NewStringUTF("null");
  }
  auto str = that->ToString(context);
  if (str.IsEmpty()) {
    noko->Throw(env, tryCatch.Exception());
    return nullptr;
  }
  v8::String::Value unicodeString(isolate, str.ToLocalChecked());
  return env->NewString(*unicodeString, unicodeString.length());
}

jstring JSValue::TypeOf(JNIEnv *env, jobject jThis) {
  SETUP(env, jThis, v8::Value)
  auto type = that->TypeOf(isolate);
  v8::String::Value unicodeString(isolate, type);
  uint16_t *unicodeChars = *unicodeString;
  return env->NewString(unicodeChars, unicodeString.length());
}

jstring JSValue::ToJson(JNIEnv *env, jobject jThis) {
  SETUP(env, jThis, v8::Value)
  v8::TryCatch tryCatch(noko->isolate_);
  auto str = v8::JSON::Stringify(context, that);
  if (str.IsEmpty()) {
    if (tryCatch.HasCaught()) {
      noko->Throw(env, tryCatch.Exception());
      return nullptr;
    }
    return env->NewString(new uint16_t[0], 0);
  }
  auto value = str.ToLocalChecked();
  v8::String::Value unicodeString(isolate, value);
  uint16_t *unicodeChars = *unicodeString;
  return env->NewString(unicodeChars, unicodeString.length());
}

jdouble JSValue::ToNumber(JNIEnv *env, jobject jThis) {
  SETUP(env, jThis, v8::Value)
  v8::TryCatch tryCatch(noko->isolate_);
  auto number = that->ToNumber(context);
  if (number.IsEmpty()) {
    noko->Throw(env, tryCatch.Exception());
    return 0.0;
  }
  v8::Local<v8::Number> checked = number.ToLocalChecked();
  return checked->Value();
}

void JSValue::Dispose(JNIEnv *env, jobject jThis) {
  auto value = JSValue::Unwrap(env, jThis);
  value->Reset();
  delete value;
  JSValue::SetReference(env, jThis, 0);
}
