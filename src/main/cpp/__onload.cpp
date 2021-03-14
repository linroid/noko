#include <__bit_reference>
//
// Created by linroid on 2019-10-19.
//
#include <jni.h>
#include <pthread.h>
#include <unistd.h>
#include <android/log.h>
#include <cmath>
#include <ares.h>
#include "NodeRuntime.h"
#include "jni/JSContext.h"
#include "jni/JSValue.h"
#include "jni/JSObject.h"
#include "jni/JSUndefined.h"
#include "jni/JSNumber.h"
#include "jni/JSString.h"
#include "jni/JSBoolean.h"
#include "jni/JSArray.h"
#include "jni/JSNull.h"
#include "jni/JSFunction.h"
#include "jni/JSError.h"
#include "jni/JSPromise.h"

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
  jmethodID onBeforeStart;
  jmethodID onBeforeExit;
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
    __android_log_write(ANDROID_LOG_ERROR, LOG_TAG, buf);
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
    __android_log_write(ANDROID_LOG_INFO, LOG_TAG, buf);
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

NodeRuntime *getRuntime(JNIEnv *env, jobject jThis) {
  jlong ptr = env->GetLongField(jThis, NodeClass.ptr);
  return reinterpret_cast<NodeRuntime *>(ptr);
}

JNICALL jint nativeStart(JNIEnv *env, jobject jThis, jobjectArray jArgs) {
  LOGD("start");
  auto runtime = getRuntime(env, jThis);
  jsize argc = env->GetArrayLength(jArgs);
  std::vector<std::string> args(static_cast<unsigned long>(argc));

  for (int i = 0; i < argc; ++i) {
    auto string = (jstring) env->GetObjectArrayElement(jArgs, i);
    const char *rawString = env->GetStringUTFChars(string, nullptr);
    args[i] = std::string(rawString);
    env->ReleaseStringUTFChars(string, rawString);
  }

  int code = jint(runtime->Start(args));
  LOGI("free runtime=%p", runtime);
  delete runtime;
  env->SetLongField(jThis, NodeClass.ptr, 0);
  return code;
}

struct PostMessage {
  jobject runnable;
  NodeRuntime *runtime;
};

JNICALL jboolean nativePost(JNIEnv *env, jobject jThis, jobject jRunnable) {
  jlong ptr = env->GetLongField(jThis, NodeClass.ptr);
  if (ptr == 0) {
    LOGE("post but ptr is 0");
    return 0;
  }
  auto *message = new PostMessage();
  message->runtime = getRuntime(env, jThis);
  message->runnable = env->NewGlobalRef(jRunnable);
  auto success = message->runtime->Post([message] {
    JNIEnv *_env;
    auto stat = message->runtime->vm_->GetEnv((void **) (&_env), JNI_VERSION_1_6);
    if (stat == JNI_EDETACHED) {
      message->runtime->vm_->AttachCurrentThread(&_env, nullptr);
    }
    _env->CallVoidMethod(message->runnable, jRunMethodId);
    _env->DeleteGlobalRef(message->runnable);
    if (stat == JNI_EDETACHED) {
      message->runtime->vm_->DetachCurrentThread();
    }
    delete message;
  });
  if (!success) {
    env->DeleteGlobalRef(message->runnable);
    delete message;
  }
  return 1;
}

JNICALL void nativeMount(JNIEnv *env, jobject jThis, jstring jSrc, jstring jDst, jint mode) {
  const char *source = env->GetStringUTFChars(jSrc, nullptr);
  const char *target = env->GetStringUTFChars(jDst, nullptr);

  auto runtime = getRuntime(env, jThis);
  runtime->Mount(source,target,  mode);

  env->ReleaseStringUTFChars(jDst, source);
  env->ReleaseStringUTFChars(jSrc, source);
}
JNICALL void nativeChroot(JNIEnv *env, jobject jThis, jstring jPath) {
  const char *path = env->GetStringUTFChars(jPath, nullptr);

  auto runtime = getRuntime(env, jThis);
  runtime->Chroot(path);

  env->ReleaseStringUTFChars(jPath, path);
}

JNICALL jlong nativeNew(JNIEnv *env, jobject jThis, jboolean keepAlive, jboolean strict) {
  auto *runtime = new NodeRuntime(env, jThis, NodeClass.onBeforeStart, NodeClass.onBeforeExit, keepAlive, strict);
  return reinterpret_cast<jlong>(runtime);
}

JNICALL void nativeExit(JNIEnv *env, jobject jThis, jint exitCode) {
  auto runtime = getRuntime(env, jThis);
  runtime->Exit(exitCode);
}
//
// char **ares_get_server_list_from_env(size_t max_servers,
//                                      size_t *num_servers) {
//     char *str = getenv("DNS_SERVERS");
//
//     if (str == nullptr || strlen(str) == 0) {
//         return nullptr;
//     }
//
//     int count = 1;
//     for (char *p = str; p[count]; p[count] == ',' ? count++ : *p++);
//     LOGI("count=%d", count);
//
//     if (!count) {
//         return nullptr;
//     }
//     if (count > max_servers) {
//         count = max_servers;
//     }
//     *num_servers = count;
//
//     char **servers;
//     servers = static_cast<char **>(malloc(sizeof(*servers) * count));
//
//     char *token;
//     int n = 0;
//     for (token = strsep(&str, ","); token != nullptr && n < count; token = strsep(&str, ","), n++) {
//         servers[n] = static_cast<char *>(malloc(sizeof(char) * 64));
//         strcpy(servers[n], token);
//     }
//
//     for (int i = 0; i < count; ++i) {
//         LOGI("dns: %s", servers[i]);
//     }
//     return servers;
// }

JNICALL void nativeSetup(__unused JNIEnv *env, __unused jclass jThis, jobject connectivity_manager) {
  ares_library_init_android(connectivity_manager);
  LOGI("ares_library_android_initialized: %d", ares_library_android_initialized());
  // int max = 5;
  // size_t num = 0;
  // ares_get_server_list_from_env(max, &num);
}

static JNINativeMethod nodeMethods[] = {
  {"nativeSetup",  "(Landroid/net/ConnectivityManager;)V",     (void *) nativeSetup},
  {"nativeNew",    "(ZZ)J",                                    (void *) nativeNew},
  {"nativeExit",   "(I)V",                                     (void *) nativeExit},
  {"nativeStart",  "([Ljava/lang/String;)I",                   (void *) nativeStart},
  {"nativeMount",  "(Ljava/lang/String;Ljava/lang/String;I)V", (void *) nativeMount},
  {"nativeChroot", "(Ljava/lang/String;)V",                    (void *) nativeChroot},
  {"nativePost",   "(Ljava/lang/Runnable;)Z",                  (void *) nativePost},
};

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *) {
  if (start_redirecting_stdout_stderr() == -1) {
    LOGE("Couldn't start redirecting stdout and stderr to logcat.");
  }
  JNIEnv *env;
  ares_library_init_jvm(vm);
  if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
    return JNI_ERR;
  }
  jclass clazz = env->FindClass("com/linroid/knode/KNode");
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
  NodeClass.onBeforeStart = env->GetMethodID(clazz, "onBeforeStart", "(Lcom/linroid/knode/js/JSContext;)V");
  NodeClass.onBeforeExit = env->GetMethodID(clazz, "onBeforeExit", "(Lcom/linroid/knode/js/JSContext;)V");

  LOAD_JNI_CLASS(JSValue)
  LOAD_JNI_CLASS(JSContext)
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
