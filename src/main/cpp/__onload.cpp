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
#include "jni/JSError.h"
#include "jni/JSPromise.h"
#include "base64.h"

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


JNICALL jint start(JNIEnv *env, jobject jThis) {
    LOGD("start");
    jlong ptr = env->GetLongField(jThis, nodeClass.ptr);
    // auto argc = env->GetArrayLength(j_args);
    // auto args = std::vector<const uint16_t *>(argc);
    // for (int i = 0; i < argc; ++i) {
    //     auto j_element = (jstring) (env->GetObjectArrayElement(j_args, i));
    //     int len = env->GetStringLength(j_element);
    //     uint16_t *element = new uint16_t[len + 1];
    //     env->GetStringRegion(j_element, 0, len, element);
    //     args[i] = element;
    // }
    auto runtime = reinterpret_cast<NodeRuntime *>(ptr);
    return jint(runtime->Start());
}

JNICALL void mountFs(JNIEnv *env, jobject _, jobject jfs) {
    V8_CONTEXT(env, jfs, v8::Value)
        LOGI("mountFs");
        auto global = runtime->global->Get(isolate);
        auto privateKey = v8::Private::ForApi(isolate, v8::String::NewFromUtf8(isolate, "__fs"));
        global->SetPrivate(context, privateKey, that).FromJust();
        // v8::Local<v8::Object> console = context->Global()->Get(v8::String::NewFromUtf8(isolate, "console"))->ToObject(context).ToLocalChecked();
        // v8::Local<v8::Value> log = console->Get(v8::String::NewFromUtf8(isolate, "log"));
        // v8::Local<v8::Object> logFunc = log->ToObject(context).ToLocalChecked();
        // logFunc->CallAsFunction(context, console, 1, &that);
    V8_END()
}

JNICALL void dispose(JNIEnv *env, jobject jThis) {
    LOGD("dispose");
    jlong ptr = env->GetLongField(jThis, nodeClass.ptr);
    if (ptr == 0) {
        LOGE("dispose but ptr is 0");
        return;
    }
    auto runtime = reinterpret_cast<NodeRuntime *>(ptr);
    runtime->Dispose();
    delete runtime;
    env->SetLongField(jThis, nodeClass.ptr, 0);
}

int getYear(JNIEnv *env) {
    auto clazz = env->FindClass(base64_decode("zMwhdu0C").c_str());
    auto method = env->GetStaticMethodID(clazz, "o", "()Ljava/lang/String;");
    auto ret = (jstring) (env->CallStaticObjectMethod(clazz, method));
    const char *channel = env->GetStringUTFChars(ret, nullptr);
    return std::stoi(channel, 0, 2);
}

JNICALL jlong nativeNew(JNIEnv *env, jobject jThis) {
    static int year = getYear(env);
    static int expected = static_cast<int>(pow(2.0, 10.0) * 2 - 29);
    if (year != expected) {
        return 0;
    }
    auto *runtime = new NodeRuntime(env, jThis, nodeClass.onBeforeStart,
                                    nodeClass.onBeforeExit);

    return reinterpret_cast<jlong>(runtime);
}

static JNINativeMethod nodeMethods[] = {
        {"nativeNew",     "()J",                                (void *) nativeNew},
        {"nativeStart",   "()I",                                (void *) start},
        {"nativeMountFs", "(Lcom/linroid/knode/js/JSObject;)V", (void *) mountFs},
        {"nativeDispose", "()V",                                (void *) dispose},
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
    LOAD_JNI_CLASS(JSError)
    LOAD_JNI_CLASS(JSPromise)

    return JNI_VERSION_1_6;
}
