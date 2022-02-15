// #include "JSContext.h"
// #include "JSUndefined.h"
// #include "JSBoolean.h"
// #include "JSNumber.h"
// #include "JSObject.h"
// #include "JSValue.h"
// #include "JSString.h"
// #include "JSError.h"
// #include <cstdint>
// #include "../Noko.h"
//
// jclass JSContext::jClazz;
// jmethodID JSContext::jConstructor;
// jfieldID JSContext::jNullId;
// jfieldID JSContext::jUndefinedId;
// jfieldID JSContext::jTrueId;
// jfieldID JSContext::jFalseId;
// jfieldID JSContext::jPtr;
//
// jint JSContext::OnLoad(JNIEnv *env) {
//   jclass clazz = env->FindClass("com/linroid/noko/types/JSContext");
//   if (clazz == nullptr) {
//     return JNI_ERR;
//   }
//   jClazz = (jclass) (env->NewGlobalRef(clazz));
//   jConstructor = env->GetMethodID(clazz, "<init>", "(JJ)V");
//   jNullId = env->GetFieldID(clazz, "sharedNull", "Lcom/linroid/noko/types/JSNull;");
//   jUndefinedId = env->GetFieldID(clazz, "sharedUndefined", "Lcom/linroid/noko/types/JSUndefined;");
//   jTrueId = env->GetFieldID(clazz, "sharedTrue", "Lcom/linroid/noko/types/JSBoolean;");
//   jFalseId = env->GetFieldID(clazz, "sharedFalse", "Lcom/linroid/noko/types/JSBoolean;");
//   jPtr = env->GetFieldID(clazz, "nokoPtr", "J");
//
//   JNINativeMethod methods[] = {
//     {"nativeEval",           "(Ljava/lang/String;Ljava/lang/String;I)Lcom/linroid/noko/types/JSValue;", (void *) Eval},
//     {"nativeParseJson",      "(Ljava/lang/String;)Lcom/linroid/noko/types/JSValue;",                    (void *) ParseJson},
//     {"nativeThrowError",     "(Ljava/lang/String;)Lcom/linroid/noko/types/JSError;",                    (void *) ThrowError},
//     {"nativeRequire",        "(Ljava/lang/String;)Lcom/linroid/noko/types/JSObject;",                   (void *) Require},
//     {"nativeClearReference", "(J)V",                                                                  (void *) ClearReference},
//   };
//
//   int rc = env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
//   if (rc != JNI_OK) {
//     return rc;
//   }
//   return JNI_OK;
// }
//

//
// JNICALL jobject JSContext::ParseJson(JNIEnv *env, jobject jThis, jstring jJson) {
//   const uint16_t *json = env->GetStringChars(jJson, nullptr);
//   const jint jsonLen = env->GetStringLength(jJson);
//   SETUP(env, jThis, v8::Object)
//   v8::Context::Scope contextScope(context);
//   v8::TryCatch tryCatch(isolate);
//   auto result = v8::JSON::Parse(context, V8_STRING(isolate, json, jsonLen));
//   if (result.IsEmpty()) {
//     noko->Throw(env, tryCatch.Exception());
//     return nullptr;
//   }
//   env->ReleaseStringChars(jJson, json);
//   return noko->ToJava(env, result.ToLocalChecked());
// }
//
// JNICALL jobject JSContext::ThrowError(JNIEnv *env, jobject jThis, jstring jMessage) {
//   const uint16_t *message = env->GetStringChars(jMessage, nullptr);
//   const jint messageLen = env->GetStringLength(jMessage);
//   SETUP(env, jThis, v8::Object)
//   auto error = v8::Exception::Error(V8_STRING(isolate, message, messageLen));
//   isolate->ThrowException(error);
//   env->ReleaseStringChars(jMessage, message);
//   return noko->ToJava(env, error);
// }
//
// // TODO: Not working, illegal context
// JNICALL jobject JSContext::Require(JNIEnv *env, jobject jThis, jstring jPath) {
//   jint pathLen = env->GetStringLength(jPath);
//   auto path = env->GetStringChars(jPath, nullptr);
//
//   SETUP(env, jThis, v8::Object)
//   auto *argv = new v8::Local<v8::Value>[1];
//   argv[0] = V8_STRING(isolate, path, pathLen);
//   auto global = noko->global_.Get(isolate);
//   v8::TryCatch tryCatch(isolate);
//
//   auto require = global->Get(context, V8_UTF_STRING(isolate, "require")).ToLocalChecked()->ToObject(context).ToLocalChecked();
//   assert(require->IsFunction());
//
//   auto result = require->CallAsFunction(context, global, 1, argv);
//   if (result.IsEmpty()) {
//     noko->Throw(env, tryCatch.Exception());
//     return nullptr;
//   }
//   env->ReleaseStringChars(jPath, path);
//   return noko->ToJava(env, result.ToLocalChecked());
// }
//
// JNICALL void JSContext::ClearReference(JNIEnv *env, jobject jThis, jlong ref) {
//   auto noko = JSValue::GetNoko(env, jThis);
//   auto reference = reinterpret_cast<v8::Persistent<v8::Value> *>(ref);
//
//   LOGV("clear reference: reference=%p, noko=%p", reference, noko);
//   if (noko == nullptr) {
//     delete reference;
//     return;
//   }
//
//   if (!noko->IsRunning()) {
//     LOGW("noko is not running, free %p", reference);
//     delete reference;
//     return;
//   }
//
//   bool submitted = noko->Post([reference, noko] {
//     if (noko->IsRunning()) {
//       v8::Locker locker(noko->isolate_);
//       v8::HandleScope handleScope(noko->isolate_);
//       reference->Reset();
//     }
//     delete reference;
//   });
//   if (!submitted) {
//     LOGW("clear(%p) but couldn't enqueue the request", reference);
//     delete reference;
//   }
// }
//
// void JSContext::SetShared(JNIEnv *env, jobject jNoko) {
//   env->SetObjectField(noko->jContext_, jNullId, noko->jNull_);
//   env->SetObjectField(noko->jContext_, jUndefinedId, noko->jUndefined_);
//   env->SetObjectField(noko->jContext_, jTrueId, noko->jTrue_);
//   env->SetObjectField(noko->jContext_, jFalseId, noko->jFalse_);
// }
