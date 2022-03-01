// #include "JSContext.h"
// #include "JsUndefined.h"
// #include "JsBoolean.h"
// #include "JsNumber.h"
// #include "JsObject.h"
// #include "JsValue.h"
// #include "JsString.h"
// #include "JsError.h"
// #include <cstdint>
// #include "../Node.h"
//
// jclass JSContext::class_;
// jmethodID JSContext::constructor;
// jfieldID JSContext::jNullId;
// jfieldID JSContext::jUndefinedId;
// jfieldID JSContext::jTrueId;
// jfieldID JSContext::jFalseId;
// jfieldID JSContext::jPtr;
//
// jint JSContext::OnLoad(JNIEnv *env) {
//   jclass class_ = env->FindClass("com/linroid/noko/types/JSContext");
//   if (class_ == nullptr) {
//     return JNI_ERR;
//   }
//   class_ = (jclass) (env->NewGlobalRef(class_));
//   constructor = env->GetMethodID(class_, "<init>", "(JJ)V");
//   jNullId = env->GetFieldID(class_, "sharedNull", "Lcom/linroid/noko/types/JsNull;");
//   jUndefinedId = env->GetFieldID(class_, "sharedUndefined", "Lcom/linroid/noko/types/JsUndefined;");
//   jTrueId = env->GetFieldID(class_, "sharedTrue", "Lcom/linroid/noko/types/JsBoolean;");
//   jFalseId = env->GetFieldID(class_, "sharedFalse", "Lcom/linroid/noko/types/JsBoolean;");
//   jPtr = env->GetFieldID(class_, "nokoPtr", "J");
//
//   JNINativeMethod methods[] = {
//     {"nativeEval",           "(Ljava/lang/String;Ljava/lang/String;I)Lcom/linroid/noko/types/JsValue;", (void *) Eval},
//     {"nativeParseJson",      "(Ljava/lang/String;)Lcom/linroid/noko/types/JsValue;",                    (void *) ParseJson},
//     {"nativeThrowError",     "(Ljava/lang/String;)Lcom/linroid/noko/types/JsError;",                    (void *) ThrowError},
//     {"nativeRequire",        "(Ljava/lang/String;)Lcom/linroid/noko/types/JsObject;",                   (void *) Require},
//     {"nativeClearReference", "(J)V",                                                                  (void *) ClearReference},
//   };
//
//   int rc = env->RegisterNatives(class_, methods, sizeof(methods) / sizeof(JNINativeMethod));
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
//     node->Throw(env, tryCatch.Exception());
//     return nullptr;
//   }
//   env->ReleaseStringChars(jJson, json);
//   return node->ToJava(env, result.ToLocalChecked());
// }
//
// JNICALL jobject JSContext::ThrowError(JNIEnv *env, jobject jThis, jstring jMessage) {
//   const uint16_t *message = env->GetStringChars(jMessage, nullptr);
//   const jint messageLen = env->GetStringLength(jMessage);
//   SETUP(env, jThis, v8::Object)
//   auto error = v8::Exception::Error(V8_STRING(isolate, message, messageLen));
//   isolate->ThrowException(error);
//   env->ReleaseStringChars(jMessage, message);
//   return node->ToJava(env, error);
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
//   auto global = node->global_.Get(isolate);
//   v8::TryCatch tryCatch(isolate);
//
//   auto require = global->Get(context, V8_UTF_STRING(isolate, "require")).ToLocalChecked()->ToObject(context).ToLocalChecked();
//   assert(require->IsFunction());
//
//   auto result = require->CallAsFunction(context, global, 1, argv);
//   if (result.IsEmpty()) {
//     node->Throw(env, tryCatch.Exception());
//     return nullptr;
//   }
//   env->ReleaseStringChars(jPath, path);
//   return node->ToJava(env, result.ToLocalChecked());
// }
//
// JNICALL void JSContext::ClearReference(JNIEnv *env, jobject jThis, jlong ref) {
//   auto noko = JsValue::GetNode(env, jThis);
//   auto reference = reinterpret_cast<v8::Persistent<v8::Value> *>(ref);
//
//   LOGV("clear reference: reference=%p, noko=%p", pointer, noko);
//   if (noko == nullptr) {
//     delete reference;
//     return;
//   }
//
//   if (!node->IsRunning()) {
//     LOGW("noko is not running, free %p", reference);
//     delete reference;
//     return;
//   }
//
//   bool submitted = node->Post([pointer, noko] {
//     if (node->IsRunning()) {
//       v8::Locker locker(node->isolate_);
//       v8::HandleScope handleScope(node->isolate_);
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
// void JSContext::SetShared(JNIEnv *env, jobject jNode) {
//   env->SetObjectField(node->jContext_, jNullId, node->jNull_);
//   env->SetObjectField(node->jContext_, jUndefinedId, node->jUndefined_);
//   env->SetObjectField(node->jContext_, jTrueId, node->jTrue_);
//   env->SetObjectField(node->jContext_, jFalseId, node->jFalse_);
// }
