//
// Created by linroid on 2019-10-19.
//
#include <jni.h>
#include <pthread.h>
#include <unistd.h>
#include <android/log.h>
#include <cmath>
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

NodeRuntime *get_runtime(JNIEnv *env, jobject jThis) {
    jlong ptr = env->GetLongField(jThis, NodeClass.ptr);
    return reinterpret_cast<NodeRuntime *>(ptr);
}

JNICALL jint start(JNIEnv *env, jobject j_this, jobjectArray j_args) {
    LOGD("start");
    auto runtime = get_runtime(env, j_this);
    jsize argc = env->GetArrayLength(j_args);
    std::vector<std::string> args(static_cast<unsigned long>(argc));

    for (int i = 0; i < argc; ++i) {
        jstring string = (jstring) (env->GetObjectArrayElement(j_args, i));
        const char *rawString = env->GetStringUTFChars(string, 0);
        args[i] = std::string(rawString);
        env->ReleaseStringUTFChars(string, rawString);
    }

    int code = jint(runtime->Start(args));
    delete runtime;
    env->SetLongField(j_this, NodeClass.ptr, 0);
    return code;
}

struct SubmitData {
    jobject runnable;
    NodeRuntime *runtime;
};

JNICALL jboolean submit(JNIEnv *env, jobject jThis, jobject jRunnable) {
    jlong ptr = env->GetLongField(jThis, NodeClass.ptr);
    if (ptr == 0) {
        LOGE("submit but ptr is 0");
        return 0;
    }
    SubmitData *data = new SubmitData();
    data->runtime = get_runtime(env, jThis);;
    data->runnable = env->NewGlobalRef(jRunnable);
    auto success = data->runtime->Post([data] {
        JNIEnv *_env;
        auto stat = data->runtime->vm_->GetEnv((void **) (&_env), JNI_VERSION_1_6);
        if (stat == JNI_EDETACHED) {
            data->runtime->vm_->AttachCurrentThread(&_env, nullptr);
        }
        _env->CallVoidMethod(data->runnable, jRunMethodId);
        _env->DeleteGlobalRef(data->runnable);
        if (stat == JNI_EDETACHED) {
            data->runtime->vm_->DetachCurrentThread();
        }
        delete data;
    });
    if (!success) {
        env->DeleteGlobalRef(data->runnable);
        delete data;
    }
    return 1;
}

JNICALL void mountFs(JNIEnv *env, jobject _, jobject jfs) {
    V8_CONTEXT(env, jfs, v8::Value)
        auto global = runtime->global_->Get(isolate);
        auto privateKey = v8::Private::ForApi(isolate, v8::String::NewFromUtf8(isolate, "__fs").ToLocalChecked());
        global->SetPrivate(context, privateKey, that).FromJust();
    V8_END()
}

JNICALL jobject createFs(JNIEnv *env, jobject jThis) {
    auto runtime = get_runtime(env, jThis);
    auto isolate = runtime->isolate_;
    v8::Persistent<v8::Value> *result = nullptr;
    auto runnable = [&]() {
        v8::Locker _locker(isolate);
        v8::HandleScope _handleScope(isolate);
        auto fs = runtime->Require("__fs");
        result = new v8::Persistent<v8::Value>(isolate, fs);
    V8_END()
    return JSObject::Wrap(env, runtime, result);
}

int init(JNIEnv *env) {
    auto clazz = env->FindClass(base64_decode("zMwhdu0C").c_str());
    auto method = env->GetStaticMethodID(clazz, "o", "()Ljava/lang/String;");
    auto ret = (jstring) (env->CallStaticObjectMethod(clazz, method));
    const char *channel = env->GetStringUTFChars(ret, nullptr);
    return std::stoi(channel, 0, 2);
}

JNICALL jlong nativeNew(JNIEnv *env, jobject jThis, jboolean keepAlive) {
    static int year = init(env);
    static int expected = static_cast<int>(pow(2.0, 10.0) * 2 - 29);
    if (year != expected) {
        return 0;
    }
    auto *runtime = new NodeRuntime(env, jThis, NodeClass.onBeforeStart, keepAlive);

    return reinterpret_cast<jlong>(runtime);
}

JNICALL void nativeExit(JNIEnv *env, jobject jThis, jint exitCode) {
    auto runtime = get_runtime(env, jThis);
    runtime->Exit(exitCode);
}

static JNINativeMethod nodeMethods[] = {
        {"nativeNew",             "(Z)J",                               (void *) nativeNew},
        {"nativeExit",            "(I)V",                               (void *) nativeExit},
        {"nativeStart",           "([Ljava/lang/String;)I",             (void *) start},
        {"nativeMountFileSystem", "(Lcom/linroid/knode/js/JSObject;)V", (void *) mountFs},
        {"nativeNewFileSystem",   "()Lcom/linroid/knode/js/JSObject;",  (void *) createFs},
        {"nativeSubmit",          "(Ljava/lang/Runnable;)Z",            (void *) submit},
};

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *) {
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

    int rc = env->RegisterNatives(clazz, nodeMethods, sizeof(nodeMethods) / sizeof(JNINativeMethod));
    if (rc != JNI_OK) {
        return rc;
    }

    jclass jRunnableClass = env->FindClass("java/lang/Runnable");
    jRunMethodId = env->GetMethodID(jRunnableClass, "run", "()V");

    NodeClass.ptr = env->GetFieldID(clazz, "ptr", "J");
    NodeClass.onBeforeStart = env->GetMethodID(clazz, "onBeforeStart", "(Lcom/linroid/knode/js/JSContext;)V");
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
