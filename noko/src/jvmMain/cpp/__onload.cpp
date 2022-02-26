#include <__bit_reference>
#include <jni.h>
#include <pthread.h>
#include <unistd.h>
#include <cmath>
#include <ares.h>
#include "EnvHelper.h"
#include "Node.h"
#include "types/JsValue.h"
#include "types/JsObject.h"
#include "types/JsUndefined.h"
#include "types/JsNumber.h"
#include "types/JsString.h"
#include "types/JsBoolean.h"
#include "types/JsArray.h"
#include "types/JsNull.h"
#include "types/JsFunction.h"
#include "types/JsError.h"
#include "types/JsPromise.h"

#define LOAD_JNI_CLASS(clazz) if (clazz::OnLoad(env) != JNI_OK) { \
    return JNI_ERR; \
}

struct JvmNodeClass {
  JNIEnv *env;
  jfieldID ptr;
  jmethodID attach;
  jmethodID detach;
} NodeClass;

jmethodID jRunMethodId;

Node *GetNoko(JNIEnv *env, jobject jThis) {
  jlong ptr = env->GetLongField(jThis, NodeClass.ptr);
  return reinterpret_cast<Node *>(ptr);
}

JNICALL jint Start(JNIEnv *env, jobject jThis, jobjectArray jArgs) {
  LOGD("start");
  auto noko = GetNoko(env, jThis);
  jsize argc = env->GetArrayLength(jArgs);
  std::vector<std::string> args(static_cast<unsigned long>(argc));

  for (int i = 0; i < argc; ++i) {
    auto string = (jstring) env->GetObjectArrayElement(jArgs, i);
    const char *rawString = env->GetStringUTFChars(string, nullptr);
    args[i] = std::string(rawString);
    env->ReleaseStringUTFChars(string, rawString);
  }

  int code = jint(noko->Start(args));
  LOGI("free noko=%p", noko);
  delete noko;
  env->SetLongField(jThis, NodeClass.ptr, 0);
  return code;
}

struct PostMessage {
  jobject runnable;
  Node *noko;
};

JNICALL jboolean Post(JNIEnv *env, jobject jThis, jobject jRunnable) {
  jlong ptr = env->GetLongField(jThis, NodeClass.ptr);
  if (ptr == 0) {
    LOGE("post but ptr is 0");
    return 0;
  }
  auto *message = new PostMessage();
  message->noko = GetNoko(env, jThis);
  message->runnable = env->NewGlobalRef(jRunnable);
  auto success = message->noko->Post([message] {
    EnvHelper _env(message->noko->vm_);
    _env->CallVoidMethod(message->runnable, jRunMethodId);
    _env->DeleteGlobalRef(message->runnable);
    delete message;
  });
  if (!success) {
    env->DeleteGlobalRef(message->runnable);
    delete message;
  }
  return 1;
}

JNICALL void MountFile(JNIEnv *env, jobject jThis, jstring jSrc, jstring jDst, jint mode) {
  const char *src = env->GetStringUTFChars(jSrc, nullptr);
  const char *dst = env->GetStringUTFChars(jDst, nullptr);

  auto noko = GetNoko(env, jThis);
  noko->MountFile(src, dst, mode);

  env->ReleaseStringUTFChars(jDst, src);
  env->ReleaseStringUTFChars(jSrc, dst);
}

JNICALL void Chroot(JNIEnv *env, jobject jThis, jstring jPath) {
  const char *path = env->GetStringUTFChars(jPath, nullptr);

  auto noko = GetNoko(env, jThis);
  noko->Chroot(path);

  env->ReleaseStringUTFChars(jPath, path);
}

JNICALL jlong New(JNIEnv *env, jobject jThis, jboolean keepAlive, jboolean strict) {
  auto *noko = new Node(env, jThis, NodeClass.attach, NodeClass.detach, keepAlive, strict);
  return reinterpret_cast<jlong>(noko);
}

JNICALL void Exit(JNIEnv *env, jobject jThis, jint exitCode) {
  auto noko = GetNoko(env, jThis);
  noko->Exit(exitCode);
}

JNICALL void Setup(__unused JNIEnv *env, __unused jclass jThis, jobject connectivity_manager) {
#if defined(__ANDROID__)
  ares_library_init_android(connectivity_manager);
  LOGI("ares_library_android_initialized: %d", ares_library_android_initialized());
#endif
  // int max = 5;
  // size_t num = 0;
  // ares_get_server_list_from_env(max, &num);
}

JNICALL jobject Eval(JNIEnv *env, jobject jThis, jstring jCode, jstring jSource, jint jLine) {
  auto noko = GetNoko(env, jThis);

  jint codeLen = env->GetStringLength(jCode);
  auto code = env->GetStringChars(jCode, nullptr);
  jint sourceLen = env->GetStringLength(jSource);
  auto source = env->GetStringChars(jSource, nullptr);

  auto result = noko->Eval(code, codeLen, source, sourceLen, jLine);
  env->ReleaseStringChars(jCode, code);
  env->ReleaseStringChars(jSource, source);
  return result;
}

JNICALL jobject ParseJson(JNIEnv *env, jobject jThis, jstring jJson) {
  const uint16_t *json = env->GetStringChars(jJson, nullptr);
  const jint jsonLen = env->GetStringLength(jJson);
  auto noko = GetNoko(env, jThis);
  auto result = noko->ParseJson(json, jsonLen);
  env->ReleaseStringChars(jJson, json);
  return result;
}

JNICALL jobject ThrowError(JNIEnv *env, jobject jThis, jstring jMessage) {
  const uint16_t *message = env->GetStringChars(jMessage, nullptr);
  const jint messageLen = env->GetStringLength(jMessage);
  auto noko = GetNoko(env, jThis);
  auto result = noko->ThrowError(message, messageLen);
  env->ReleaseStringChars(jMessage, message);
  return result;
}

// TODO: Not working, illegal context
JNICALL jobject Require(JNIEnv *env, jobject jThis, jstring jPath) {
  jint pathLen = env->GetStringLength(jPath);
  auto path = env->GetStringChars(jPath, nullptr);
  auto noko = GetNoko(env, jThis);
  auto result = noko->Require(path, pathLen);
  env->ReleaseStringChars(jPath, path);
  return result;
}

JNICALL void ClearReference(JNIEnv *env, jobject jThis, jlong ref) {
  auto noko = JsValue::GetNode(env, jThis);
  auto reference = reinterpret_cast<v8::Persistent<v8::Value> *>(ref);

  LOGV("clear reference: reference=%p, noko=%p", reference, noko);
  if (noko == nullptr) {
    delete reference;
    return;
  }

  if (!noko->IsRunning()) {
    LOGW("noko is not running, free %p", reference);
    delete reference;
    return;
  }

  bool submitted = noko->Post([reference, noko] {
    if (noko->IsRunning()) {
      v8::Locker locker(noko->isolate_);
      v8::HandleScope handleScope(noko->isolate_);
      reference->Reset();
    }
    delete reference;
  });
  if (!submitted) {
    LOGW("clear(%p) but couldn't enqueue the request", reference);
    delete reference;
  }
}

static JNINativeMethod nodeMethods[] = {
#if defined(__ANDROID__)
    {"nativeSetup",          "(Landroid/net/ConnectivityManager;)V",                                    (void *) Setup},
#endif
    {"nativeNew",            "(ZZ)J",                                                                   (void *) New},
    {"nativeExit",           "(I)V",                                                                    (void *) Exit},
    {"nativeStart",          "([Ljava/lang/String;)I",                                                  (void *) Start},
    {"nativeMountFile",      "(Ljava/lang/String;Ljava/lang/String;I)V",                                (void *) MountFile},
    {"nativeChroot",         "(Ljava/lang/String;)V",                                                   (void *) Chroot},
    {"nativePost",           "(Lkotlinx/coroutines/Runnable;)Z",                                        (void *) Post},
    {"nativeEval",           "(Ljava/lang/String;Ljava/lang/String;I)Lcom/linroid/noko/types/JsValue;", (void *) Eval},
    {"nativeParseJson",      "(Ljava/lang/String;)Lcom/linroid/noko/types/JsValue;",                    (void *) ParseJson},
    {"nativeThrowError",     "(Ljava/lang/String;)Lcom/linroid/noko/types/JsError;",                    (void *) ThrowError},
    {"nativeRequire",        "(Ljava/lang/String;)Lcom/linroid/noko/types/JsObject;",                   (void *) Require},
    {"nativeClearReference", "(J)V",                                                                    (void *) ClearReference},
};

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *) {
  JNIEnv *env = nullptr;
#if defined(__ANDROID__)
  ares_library_init_jvm(vm);
#endif
  if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
    return JNI_ERR;
  }
  jclass clazz = env->FindClass("com/linroid/noko/Node");
  if (clazz == nullptr) {
    return JNI_ERR;
  }

  int rc = env->RegisterNatives(clazz, nodeMethods, sizeof(nodeMethods) / sizeof(JNINativeMethod));
  if (rc != JNI_OK) {
    return rc;
  }

  jclass jRunnableClass = env->FindClass("java/lang/Runnable");
  jRunMethodId = env->GetMethodID(jRunnableClass, "run", "()V");

  NodeClass.ptr = env->GetFieldID(clazz, "ptr", "J");
  NodeClass.attach = env->GetMethodID(clazz, "attach", "(Lcom/linroid/noko/types/JsObject;)V");
  NodeClass.detach = env->GetMethodID(clazz, "detach", "(Lcom/linroid/noko/types/JsObject;)V");

  LOAD_JNI_CLASS(Node)
  LOAD_JNI_CLASS(JsValue)
  LOAD_JNI_CLASS(JsObject)
  LOAD_JNI_CLASS(JsBoolean)
  LOAD_JNI_CLASS(JsNumber)
  LOAD_JNI_CLASS(JsString)
  LOAD_JNI_CLASS(JsArray)
  LOAD_JNI_CLASS(JsUndefined)
  LOAD_JNI_CLASS(JsNull)
  LOAD_JNI_CLASS(JsFunction)
  LOAD_JNI_CLASS(JsError)
  LOAD_JNI_CLASS(JsPromise)

  return JNI_VERSION_1_6;
}
