//
// Created by linroid on 2019-10-19.
//
#include <jni.h>
#include <pthread.h>
#include <unistd.h>
#include <android/log.h>
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
} nodeClass;

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
    return 0;
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
    return 0;
}

int start_redirecting_stdout_stderr() {
    //set stdout as unbuffered.
    setvbuf(stdout, 0, _IONBF, 0);
    pipe(pipe_stdout);
    dup2(pipe_stdout[1], STDOUT_FILENO);

    //set stderr as unbuffered.
    setvbuf(stderr, 0, _IONBF, 0);
    pipe(pipe_stderr);
    dup2(pipe_stderr[1], STDERR_FILENO);

    if (pthread_create(&thread_stdout, 0, thread_stdout_func, 0) == -1)
        return -1;
    pthread_detach(thread_stdout);

    if (pthread_create(&thread_stderr, 0, thread_stderr_func, 0) == -1)
        return -1;
    pthread_detach(thread_stderr);

    return 0;
}


JNICALL jint start(JNIEnv *env, jobject jthis) {
    LOGD("start");
    jlong ptr = env->GetLongField(jthis, nodeClass.ptr);
    // auto argc = env->GetArrayLength(j_args);
    // auto args = std::vector<const uint16_t *>(argc);
    // for (int i = 0; i < argc; ++i) {
    //     auto j_element = (jstring) (env->GetObjectArrayElement(j_args, i));
    //     int len = env->GetStringLength(j_element);
    //     uint16_t *element = new uint16_t[len + 1];
    //     env->GetStringRegion(j_element, 0, len, element);
    //     args[i] = element;
    // }
    auto node = reinterpret_cast<NodeRuntime *>(ptr);
    return jint(node->Start());
}

JNICALL void setFs(JNIEnv *env, jobject jthis, jlong fsPtr) {
    LOGD("setFs");
    jlong ptr = env->GetLongField(jthis, nodeClass.ptr);
    auto node = reinterpret_cast<NodeRuntime *>(ptr);
}

JNICALL void dispose(JNIEnv *env, jobject jthis) {
    LOGD("dispose");
    jlong ptr = env->GetLongField(jthis, nodeClass.ptr);
    if (ptr == 0) {
        LOGE("dispose but ptr is 0");
        return;
    }
    auto node = reinterpret_cast<NodeRuntime *>(ptr);
    node->Dispose();
    delete node;
    env->SetLongField(jthis, nodeClass.ptr, 0);
}

JNICALL jlong nativeNew(JNIEnv *env, jobject jthis) {
    LOGD("nativeNew");
    auto *node = new NodeRuntime(env, jthis, nodeClass.onBeforeStart,
                                 nodeClass.onBeforeExit);
    return reinterpret_cast<jlong>(node);
}

static JNINativeMethod nodeMethods[] = {
        {"nativeNew",    "()J",  (void *) nativeNew},
        {"nativeStart",   "()I",  (void *) start},
        {"nativeSetFs",   "(J)V", (void *) setFs},
        {"nativeDispose", "()V",  (void *) dispose},
};


JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *) {
    LOGI("JNI_OnLoad");
    if (start_redirecting_stdout_stderr() == -1) {
        LOGE("Couldn't start redirecting stdout and stderr to logcat.");
    }

    JNIEnv *env;
    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }
    jclass clazz = env->FindClass("com/linroid/knode/KNode");
    if (clazz == nullptr) {
        return JNI_ERR;
    }

    int rc = env->RegisterNatives(clazz, nodeMethods,
                                  sizeof(nodeMethods) / sizeof(JNINativeMethod));
    if (rc != JNI_OK) {
        return rc;
    }
    nodeClass.ptr = env->GetFieldID(clazz, "ptr", "J");
    nodeClass.onBeforeStart = env->GetMethodID(clazz, "onBeforeStart",
                                               "(Lcom/linroid/knode/js/JSContext;)V");
    nodeClass.onBeforeExit = env->GetMethodID(clazz, "onBeforeExit", "(I)V");

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

    return JNI_VERSION_1_6;
}
