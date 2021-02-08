#include <stdint.h>
//
// Created by linroid on 2019-10-20.
//

#include "JSContext.h"
#include "JSUndefined.h"
#include "JSBoolean.h"
#include "JSNumber.h"
#include "JSObject.h"
#include "JSValue.h"
#include "JSString.h"
#include "../NodeRuntime.h"
#include "JSError.h"

jclass JSContext::jClazz;
jmethodID JSContext::jConstructor;
jfieldID JSContext::jNullId;
jfieldID JSContext::jUndefinedId;
jfieldID JSContext::jTrueId;
jfieldID JSContext::jFalseId;

jint JSContext::OnLoad(JNIEnv *env) {
  jclass clazz = env->FindClass("com/linroid/knode/js/JSContext");
  if (clazz == nullptr) {
    return JNI_ERR;
  }
  jClazz = (jclass) (env->NewGlobalRef(clazz));
  jConstructor = env->GetMethodID(clazz, "<init>", "(JJ)V");
  jNullId = env->GetFieldID(clazz, "sharedNull", "Lcom/linroid/knode/js/JSNull;");
  jUndefinedId = env->GetFieldID(clazz, "sharedUndefined", "Lcom/linroid/knode/js/JSUndefined;");
  jTrueId = env->GetFieldID(clazz, "sharedTrue", "Lcom/linroid/knode/js/JSBoolean;");
  jFalseId = env->GetFieldID(clazz, "sharedFalse", "Lcom/linroid/knode/js/JSBoolean;");

  JNINativeMethod methods[] = {
    {"nativeEval",           "(Ljava/lang/String;Ljava/lang/String;I)Lcom/linroid/knode/js/JSValue;", (void *) Eval},
    {"nativeParseJson",      "(Ljava/lang/String;)Lcom/linroid/knode/js/JSValue;",                    (void *) ParseJson},
    {"nativeThrowError",     "(Ljava/lang/String;)Lcom/linroid/knode/js/JSError;",                    (void *) ThrowError},
    {"nativeRequire",        "(Ljava/lang/String;)Lcom/linroid/knode/js/JSObject;",                   (void *) Require},
    {"nativeClearReference", "(J)V",                                                                  (void *) ClearReference},
  };

  int rc = env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
  if (rc != JNI_OK) {
    return rc;
  }
  return JNI_OK;
}

jobject JSContext::Eval(JNIEnv *env, jobject jThis, jstring jCode, jstring jSource, jint jLine) {
  jint codeLen = env->GetStringLength(jCode);
  auto code = env->GetStringChars(jCode, nullptr);

  jint sourceLen = env->GetStringLength(jSource);
  auto source = env->GetStringChars(jSource, nullptr);

  SETUP(env, jThis, v8::Object)
  v8::TryCatch tryCatch(runtime->isolate_);
  v8::ScriptOrigin scriptOrigin(V8_STRING(isolate, source, sourceLen), v8::Integer::New(runtime->isolate_, jLine));
  auto script = v8::Script::Compile(context, V8_STRING(isolate, code, codeLen), &scriptOrigin);
  if (script.IsEmpty()) {
    LOGE("Compile script with an exception");
    runtime->Throw(env, tryCatch.Exception());
    return nullptr;
  }
  auto result = script.ToLocalChecked()->Run(context);
  if (result.IsEmpty()) {
    LOGE("Run script with an exception");
    runtime->Throw(env, tryCatch.Exception());
    return nullptr;
  }
  env->ReleaseStringChars(jCode, code);
  env->ReleaseStringChars(jSource, source);
  return runtime->ToJava(env, result.ToLocalChecked());
}

jobject JSContext::ParseJson(JNIEnv *env, jobject jThis, jstring jJson) {
  const uint16_t *json = env->GetStringChars(jJson, nullptr);
  const jint jsonLen = env->GetStringLength(jJson);
  SETUP(env, jThis, v8::Object)
  v8::Context::Scope contextScope(context);
  v8::TryCatch tryCatch(isolate);
  auto result = v8::JSON::Parse(context, V8_STRING(isolate, json, jsonLen));
  if (result.IsEmpty()) {
    runtime->Throw(env, tryCatch.Exception());
    return nullptr;
  }
  env->ReleaseStringChars(jJson, json);
  return runtime->ToJava(env, result.ToLocalChecked());
}

jobject JSContext::ThrowError(JNIEnv *env, jobject jThis, jstring jMessage) {
  const uint16_t *message = env->GetStringChars(jMessage, nullptr);
  const jint messageLen = env->GetStringLength(jMessage);
  SETUP(env, jThis, v8::Object)
  auto error = v8::Exception::Error(V8_STRING(isolate, message, messageLen));
  isolate->ThrowException(error);
  env->ReleaseStringChars(jMessage, message);
  return runtime->ToJava(env, error);
}

// TODO: Not working, illegal context
jobject JSContext::Require(JNIEnv *env, jobject jThis, jstring jPath) {
  jint pathLen = env->GetStringLength(jPath);
  auto path = env->GetStringChars(jPath, nullptr);

  SETUP(env, jThis, v8::Object)
  auto *argv = new v8::Local<v8::Value>[1];
  argv[0] = V8_STRING(isolate, path, pathLen);
  auto global = runtime->global_->Get(isolate);
  v8::TryCatch tryCatch(isolate);

  auto require = global->Get(context, V8_UTF_STRING(isolate, "require")).ToLocalChecked()->ToObject(context).ToLocalChecked();
  assert(require->IsFunction());

  auto result = require->CallAsFunction(context, global, 1, argv);
  if (result.IsEmpty()) {
    runtime->Throw(env, tryCatch.Exception());
    return nullptr;
  }
  env->ReleaseStringChars(jPath, path);
  return runtime->ToJava(env, result.ToLocalChecked());
}

void JSContext::ClearReference(JNIEnv *env, jobject jThis, jlong ref) {
  auto reference = reinterpret_cast<v8::Persistent<v8::Value> *>(ref);
  auto runtime = JSValue::GetRuntime(env, jThis);
  LOGV("clear %p(runtime=%p)", reference, runtime);
  if (runtime == nullptr) {
    LOGW("delete %p(runtime=%p) @1", reference, runtime);
    delete reference;
    return;
  }
  bool submitted = runtime->Post([reference, runtime] {
    if (runtime != nullptr && runtime->IsRunning()) {
      v8::Locker locker(runtime->isolate_);
      v8::HandleScope handleScope(runtime->isolate_);
      reference->Reset();
      LOGI("delete %p(runtime=%p) @2", reference, runtime);
    } else {
      LOGW("delete %p(runtime=%p) @3", reference, runtime);
    }
    delete reference;
  });
  if (!submitted) {
    LOGW("delete %p(runtime=%p) @4", reference, runtime);
    delete reference;
  }
}

void JSContext::SetShared(JNIEnv *env, NodeRuntime *runtime) {
  env->SetObjectField(runtime->jContext_, jNullId, runtime->jNull_);
  env->SetObjectField(runtime->jContext_, jUndefinedId, runtime->jUndefined_);
  env->SetObjectField(runtime->jContext_, jTrueId, runtime->jTrue_);
  env->SetObjectField(runtime->jContext_, jFalseId, runtime->jFalse_);
}
