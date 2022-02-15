#include <__bit_reference>
#include <jni.h>
#include <pthread.h>
#include <unistd.h>
#include <android/log.h>
#include <cmath>
#include <ares.h>
#include "Noko.h"
#include "types/JSContext.h"
#include "types/JSValue.h"
#include "types/JSObject.h"
#include "types/JSUndefined.h"
#include "types/JSNumber.h"
#include "types/JSString.h"
#include "types/JSBoolean.h"
#include "types/JSArray.h"
#include "types/JSNull.h"
#include "types/JSFunction.h"
#include "types/JSError.h"
#include "types/JSPromise.h"

#define LOAD_JNI_CLASS(clazz) if (clazz::OnLoad(env) != JNI_OK) { \
    return JNI_ERR; \
}

int pipe_stdout[2];
int pipe_stderr[2];
pthread_t thread_stdout;
pthread_t thread_stderr;

struct JvmNodeClass {
  JNIEnv *env;
  jfieldID ptr;
  jmethodID attach;
  jmethodID detach;
} NodeClass;

jmethodID jRunMethodId;

void *thread_stderr_func(void *) {
  ssize_t redirect_size;
  char buf[2048];
  while ((redirect_size = read(pipe_stderr[0], buf, sizeof buf - 1)) > 0) {
    //__android_log will add a new line anyway.
    if (buf[redirect_size - 1] == '\n')
      --redirect_size;
    buf[redirect_size] = 0;
    __android_log_write(ANDROID_LOG_ERROR, "stderr", buf);
  }
  return nullptr;
}

void *thread_stdout_func(void *) {
  ssize_t redirect_size;
  char buf[2048];
  while ((redirect_size = read(pipe_stdout[0], buf, sizeof buf - 1)) > 0) {
    //__android_log will add a new line anyway.
    if (buf[redirect_size - 1] == '\n')
      --redirect_size;
    buf[redirect_size] = 0;
    __android_log_write(ANDROID_LOG_INFO, "stdout", buf);
  }
  return nullptr;
}

int start_redirecting_stdout_stderr() {
  //set stdout as unbuffered.
  setvbuf(stdout, nullptr, _IONBF, 0);
  pipe(pipe_stdout);
  dup2(pipe_stdout[1], STDOUT_FILENO);

  //set stderr as unbuffered.
  setvbuf(stderr, nullptr, _IONBF, 0);
  pipe(pipe_stderr);
  dup2(pipe_stderr[1], STDERR_FILENO);

  if (pthread_create(&thread_stdout, nullptr, thread_stdout_func, nullptr) == -1)
    return -1;
  pthread_detach(thread_stdout);

  if (pthread_create(&thread_stderr, nullptr, thread_stderr_func, nullptr) == -1)
    return -1;
  pthread_detach(thread_stderr);

  return 0;
}

Noko *GetNoko(JNIEnv *env, jobject jThis) {
  jlong ptr = env->GetLongField(jThis, NodeClass.ptr);
  return reinterpret_cast<Noko *>(ptr);
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
  Noko *noko;
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
    JNIEnv *_env;
    auto stat = message->noko->vm_->GetEnv((void **) (&_env), JNI_VERSION_1_6);
    if (stat == JNI_EDETACHED) {
      message->noko->vm_->AttachCurrentThread(&_env, nullptr);
    }
    _env->CallVoidMethod(message->runnable, jRunMethodId);
    _env->DeleteGlobalRef(message->runnable);
    if (stat == JNI_EDETACHED) {
      message->noko->vm_->DetachCurrentThread();
    }
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
  auto *noko = new Noko(env, jThis, NodeClass.attach, NodeClass.detach, keepAlive, strict);
  return reinterpret_cast<jlong>(noko);
}

JNICALL void Exit(JNIEnv *env, jobject jThis, jint exitCode) {
  auto noko = GetNoko(env, jThis);
  noko->Exit(exitCode);
}

JNICALL void Setup(__unused JNIEnv *env, __unused jclass jThis, jobject connectivity_manager) {
  ares_library_init_android(connectivity_manager);
  LOGI("ares_library_android_initialized: %d", ares_library_android_initialized());
  // int max = 5;
  // size_t num = 0;
  // ares_get_server_list_from_env(max, &num);
}

JNICALL jobject Eval(JNIEnv *env, jobject jThis, jstring jCode, jstring jSource, jint jLine) {
  jint codeLen = env->GetStringLength(jCode);
  auto code = env->GetStringChars(jCode, nullptr);

  jint sourceLen = env->GetStringLength(jSource);
  auto source = env->GetStringChars(jSource, nullptr);

  SETUP(env, jThis, v8::Object)
  v8::TryCatch tryCatch(noko->isolate_);
  v8::ScriptOrigin scriptOrigin(V8_STRING(isolate, source, sourceLen), v8::Integer::New(noko->isolate_, jLine), v8::Integer::New(noko->isolate_, 0));
  auto script = v8::Script::Compile(context, V8_STRING(isolate, code, codeLen), &scriptOrigin);
  if (script.IsEmpty()) {
    LOGE("Compile script with an exception");
    noko->Throw(env, tryCatch.Exception());
    return nullptr;
  }
  auto result = script.ToLocalChecked()->Run(context);
  if (result.IsEmpty()) {
    LOGE("Run script with an exception");
    noko->Throw(env, tryCatch.Exception());
    return nullptr;
  }
  env->ReleaseStringChars(jCode, code);
  env->ReleaseStringChars(jSource, source);
  return noko->ToJava(env, result.ToLocalChecked());
}


JNICALL jobject ParseJson(JNIEnv *env, jobject jThis, jstring jJson) {
  const uint16_t *json = env->GetStringChars(jJson, nullptr);
  const jint jsonLen = env->GetStringLength(jJson);
  SETUP(env, jThis, v8::Object)
  v8::Context::Scope contextScope(context);
  v8::TryCatch tryCatch(isolate);
  auto result = v8::JSON::Parse(context, V8_STRING(isolate, json, jsonLen));
  if (result.IsEmpty()) {
    noko->Throw(env, tryCatch.Exception());
    return nullptr;
  }
  env->ReleaseStringChars(jJson, json);
  return noko->ToJava(env, result.ToLocalChecked());
}

JNICALL jobject ThrowError(JNIEnv *env, jobject jThis, jstring jMessage) {
  const uint16_t *message = env->GetStringChars(jMessage, nullptr);
  const jint messageLen = env->GetStringLength(jMessage);
  SETUP(env, jThis, v8::Object)
  auto error = v8::Exception::Error(V8_STRING(isolate, message, messageLen));
  isolate->ThrowException(error);
  env->ReleaseStringChars(jMessage, message);
  return noko->ToJava(env, error);
}

// TODO: Not working, illegal context
JNICALL jobject Require(JNIEnv *env, jobject jThis, jstring jPath) {
  jint pathLen = env->GetStringLength(jPath);
  auto path = env->GetStringChars(jPath, nullptr);

  SETUP(env, jThis, v8::Object)
  auto *argv = new v8::Local<v8::Value>[1];
  argv[0] = V8_STRING(isolate, path, pathLen);
  auto global = noko->global_.Get(isolate);
  v8::TryCatch tryCatch(isolate);

  auto require = global->Get(context, V8_UTF_STRING(isolate, "require")).ToLocalChecked()->ToObject(context).ToLocalChecked();
  assert(require->IsFunction());

  auto result = require->CallAsFunction(context, global, 1, argv);
  if (result.IsEmpty()) {
    noko->Throw(env, tryCatch.Exception());
    return nullptr;
  }
  env->ReleaseStringChars(jPath, path);
  return noko->ToJava(env, result.ToLocalChecked());
}

JNICALL void ClearReference(JNIEnv *env, jobject jThis, jlong ref) {
  auto noko = JSValue::GetNoko(env, jThis);
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
  {"nativeSetup",           "(Landroid/net/ConnectivityManager;)V",                                     (void *) Setup},
  {"nativeNew",             "(ZZ)J",                                                                    (void *) New},
  {"nativeExit",            "(I)V",                                                                     (void *) Exit},
  {"nativeStart",           "([Ljava/lang/String;)I",                                                   (void *) Start},
  {"nativeMountFile",       "(Ljava/lang/String;Ljava/lang/String;I)V",                                 (void *) MountFile},
  {"nativeChroot",          "(Ljava/lang/String;)V",                                                    (void *) Chroot},
  {"nativePost",            "(Ljava/lang/Runnable;)Z",                                                  (void *) Post},
  {"nativeEval",            "(Ljava/lang/String;Ljava/lang/String;I)Lcom/linroid/noko/types/JSValue;",  (void *) Eval},
  {"nativeParseJson",       "(Ljava/lang/String;)Lcom/linroid/noko/types/JSValue;",                     (void *) ParseJson},
  {"nativeThrowError",      "(Ljava/lang/String;)Lcom/linroid/noko/types/JSError;",                     (void *) ThrowError},
  {"nativeRequire",         "(Ljava/lang/String;)Lcom/linroid/noko/types/JSObject;",                    (void *) Require},
  {"nativeClearReference",  "(J)V",                                                                     (void *) ClearReference},
};

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *) {
  if (start_redirecting_stdout_stderr() == -1) {
    LOGE("Couldn't start redirecting stdout and stderr to logcat.");
  }
  JNIEnv *env = nullptr;
  ares_library_init_jvm(vm);
  if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
    return JNI_ERR;
  }
  jclass clazz = env->FindClass("com/linroid/noko/Noko");
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
  NodeClass.attach = env->GetMethodID(clazz, "attach", "(Lcom/linroid/noko/types/JSContext;)V");
  NodeClass.detach = env->GetMethodID(clazz, "detach", "(Lcom/linroid/noko/types/JSContext;)V");

  Noko::OnLoad(env);

  LOAD_JNI_CLASS(JSValue)
  LOAD_JNI_CLASS(JSObject)
  LOAD_JNI_CLASS(JSBoolean)
  LOAD_JNI_CLASS(JSNumber)
  LOAD_JNI_CLASS(JSString)
  LOAD_JNI_CLASS(JSArray)
  LOAD_JNI_CLASS(JSUndefined)
  LOAD_JNI_CLASS(JSNull)
  LOAD_JNI_CLASS(JSFunction)
  LOAD_JNI_CLASS(JSError)
  LOAD_JNI_CLASS(JSPromise)

  return JNI_VERSION_1_6;
}
