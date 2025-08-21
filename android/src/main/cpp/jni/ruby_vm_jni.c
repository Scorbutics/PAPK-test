#include <stdlib.h>
#include <string.h>
#include <logging.h>

#include <android/log.h>

#include "env.h"
#include "ruby_vm_jni.h"
#include "ruby-vm.h"
#include "ruby-script-location.h"
#include "ruby-script.h"
#include "ruby-interpreter.h"
#include "completion-task.h"

// Global context for callbacks (you might want to make this per-interpreter)
static JNICallbackContext* g_callback_context = NULL;

// Completion callback context
typedef struct {
    JavaVM* jvm;
    jobject callback_obj;
    jmethodID invoke_method_id;
} CompletionCallbackContext;


// Global context completion callback (in real implementation, you might want per-operation storage)
static CompletionCallbackContext* g_completion_context = NULL;

// Helper function to create JNI callback context
static JNICallbackContext* create_jni_context(JNIEnv* env, jobject kotlin_listener) {
    JNICallbackContext* context = malloc(sizeof(JNICallbackContext));
    if (!context) return NULL;

    // Get JavaVM for later use in callbacks
    if ((*env)->GetJavaVM(env, &context->jvm) != JNI_OK) {
        free(context);
        return NULL;
    }

    // Create global reference to Kotlin listener object
    context->kotlin_listener = (*env)->NewGlobalRef(env, kotlin_listener);
    if (!context->kotlin_listener) {
        free(context);
        return NULL;
    }

    // Get the class and method IDs
    jclass listener_class = (*env)->GetObjectClass(env, kotlin_listener);
    if (!listener_class) {
        (*env)->DeleteGlobalRef(env, context->kotlin_listener);
        free(context);
        return NULL;
    }

    // Get method IDs for the LogListener interface methods
    context->accept_method_id = (*env)->GetMethodID(env, listener_class,
                                                   "accept", "(Ljava/lang/String;)V");
    context->error_method_id = (*env)->GetMethodID(env, listener_class,
                                                  "onLogError", "(Ljava/lang/String;)V");

    if (!context->accept_method_id || !context->error_method_id) {
        (*env)->DeleteGlobalRef(env, context->kotlin_listener);
        free(context);
        return NULL;
    }

    return context;
}

static void destroy_jni_context(JNICallbackContext* context) {
    if (!context) return;

    JNIEnv* env;
    if ((*context->jvm)->AttachCurrentThread(context->jvm, &env, NULL) == JNI_OK) {
        (*env)->DeleteGlobalRef(env, context->kotlin_listener);
        (*context->jvm)->DetachCurrentThread(context->jvm);
    }

    free(context);
}

// C callback functions that will call into Kotlin
static void jni_log_accept_callback(const char* message) {
    if (!g_callback_context || !message) return;

    JNIEnv* env;
    if ((*g_callback_context->jvm)->AttachCurrentThread(g_callback_context->jvm, &env, NULL) != JNI_OK) {
        return;
    }

    // Convert C string to Java string
    jstring j_message = (*env)->NewStringUTF(env, message);
    if (j_message) {
        // Call the Kotlin accept method
        (*env)->CallVoidMethod(env, g_callback_context->kotlin_listener,
                              g_callback_context->accept_method_id, j_message);

        // Clean up local reference
        (*env)->DeleteLocalRef(env, j_message);
    }

    // Check for exceptions
    if ((*env)->ExceptionCheck(env)) {
        (*env)->ExceptionClear(env);
    }

    (*g_callback_context->jvm)->DetachCurrentThread(g_callback_context->jvm);
}

static void jni_log_error_callback(const char* error_message) {
    if (!g_callback_context || !error_message) return;

    JNIEnv* env;
    if ((*g_callback_context->jvm)->AttachCurrentThread(g_callback_context->jvm, &env, NULL) != JNI_OK) {
        return;
    }

    // Convert C string to Java string
    jstring j_error_message = (*env)->NewStringUTF(env, error_message);
    if (j_error_message) {
        // Call the Kotlin onLogError method
        (*env)->CallVoidMethod(env, g_callback_context->kotlin_listener,
                              g_callback_context->error_method_id, j_error_message);

        // Clean up local reference
        (*env)->DeleteLocalRef(env, j_error_message);
    }

    // Check for exceptions
    if ((*env)->ExceptionCheck(env)) {
        (*env)->ExceptionClear(env);
    }

    (*g_callback_context->jvm)->DetachCurrentThread(g_callback_context->jvm);
}

// Helper to convert jstring to C string
char* jstring_to_cstring(JNIEnv* env, jstring j_str) {
    if (!j_str) return NULL;

    const char* str = (*env)->GetStringUTFChars(env, j_str, NULL);
    if (!str) return NULL;

    char* c_str = strdup(str);
    (*env)->ReleaseStringUTFChars(env, j_str, str);

    return c_str;
}



JNIEXPORT jlong JNICALL
Java_com_scorbutics_rubyvm_RubyVMNative_createInterpreter(JNIEnv *env, jclass clazz,
                                                     jstring app_path,
                                                     jstring ruby_base_directory,
                                                     jstring native_libs_directory,
                                                     jobject kotlin_listener) {
    (void) clazz;
    char* c_app_path = jstring_to_cstring(env, app_path);
    char* c_ruby_base_directory = jstring_to_cstring(env, ruby_base_directory);
    char* c_native_libs_directory = jstring_to_cstring(env, native_libs_directory);

    LoggingSetNativeLoggingFunction(__android_log_write);

    // Create JNI callback context
    JNICallbackContext* callback_context = create_jni_context(env, kotlin_listener);
    if (!callback_context) {
        free(c_app_path);
        return 0; // null pointer
    }

    // Store globally for callbacks (you might want per-interpreter storage)
    g_callback_context = callback_context;

    // Create LogListener with C callback functions
    LogListener listener = {
        .context = callback_context,
        .accept = jni_log_accept_callback,
        .on_log_error = jni_log_error_callback
    };

    // Create interpreter
    RubyInterpreter* interpreter = ruby_interpreter_create(c_app_path, c_ruby_base_directory, c_native_libs_directory, listener);

    free(c_ruby_base_directory);
    free(c_native_libs_directory);
    free(c_app_path);

    return (jlong)interpreter;
}

JNIEXPORT void JNICALL
Java_com_scorbutics_rubyvm_RubyVMNative_destroyInterpreter(JNIEnv *env, jclass clazz,
                                                      jlong interpreter_ptr) {
    (void) env;
    (void) clazz;
    RubyInterpreter* interpreter = (RubyInterpreter*)interpreter_ptr;
    ruby_interpreter_destroy(interpreter);

    // Clean up callback context
    if (g_callback_context) {
        destroy_jni_context(g_callback_context);
        g_callback_context = NULL;
    }
}

JNIEXPORT jlong JNICALL
Java_com_scorbutics_rubyvm_RubyVMNative_createScript(JNIEnv *env, jclass clazz,
                                                jstring content) {
    (void) clazz;
    char* c_content = jstring_to_cstring(env, content);
    if (!c_content) return 0;

    RubyScript* script = ruby_script_create_from_content(c_content);
    free(c_content);

    return (jlong)script;
}

JNIEXPORT void JNICALL
Java_com_scorbutics_rubyvm_RubyVMNative_destroyScript(JNIEnv *env, jclass clazz,
                                                 jlong script_ptr) {
    (void) env;
    (void) clazz;
    RubyScript* script = (RubyScript*)script_ptr;
    ruby_script_destroy(script);
}

void jni_completion_callback(int result) {
    if (!g_completion_context) return;

    JNIEnv* env;
    if ((*g_completion_context->jvm)->AttachCurrentThread(g_completion_context->jvm, &env, NULL) != JNI_OK) {
        return;
    }

    // Call the Kotlin callback function with the result
    (*env)->CallVoidMethod(env, g_completion_context->callback_obj,
                          g_completion_context->invoke_method_id, (jint)result);

    // Check for exceptions
    if ((*env)->ExceptionCheck(env)) {
        (*env)->ExceptionClear(env);
    }

    (*g_completion_context->jvm)->DetachCurrentThread(g_completion_context->jvm);
}

// Helper functions for completion callback context
static CompletionCallbackContext* create_completion_context(JNIEnv* env, jobject completion_callback, int* errorCode) {

    CompletionCallbackContext* context = malloc(sizeof(CompletionCallbackContext));
    if (!context) {
        *errorCode = 1;
        return NULL;
    }

    // Get JavaVM for later use in callbacks
    if ((*env)->GetJavaVM(env, &context->jvm) != JNI_OK) {
        free(context);
        *errorCode = 2;
        return NULL;
    }

    // Create global reference to callback object
    context->callback_obj = (*env)->NewGlobalRef(env, completion_callback);
    if (!context->callback_obj) {
        free(context);
        *errorCode = 3;
        return NULL;
    }

    // Get the class and method ID for the callback
    jclass callback_class = (*env)->GetObjectClass(env, completion_callback);
    if (!callback_class) {
        (*env)->DeleteGlobalRef(env, context->callback_obj);
        free(context);
        *errorCode = 4;
        return NULL;
    }

    // Look for invoke method that takes an int parameter
    context->invoke_method_id = (*env)->GetMethodID(env, callback_class,
                                                   "complete", "(I)V");
    if (!context->invoke_method_id) {
        (*env)->DeleteGlobalRef(env, context->callback_obj);
        free(context);
        *errorCode = 5;
        return NULL;
    }

    *errorCode = 0;
    return context;
}

static void destroy_completion_context(CompletionCallbackContext* context) {
    if (!context) return;

    JNIEnv* env;
    if ((*context->jvm)->AttachCurrentThread(context->jvm, &env, NULL) == JNI_OK) {
        (*env)->DeleteGlobalRef(env, context->callback_obj);
        (*context->jvm)->DetachCurrentThread(context->jvm);
    }

    free(context);
}


JNIEXPORT void JNICALL
Java_com_scorbutics_rubyvm_RubyVMNative_enqueueScript(JNIEnv *env, jclass clazz,
                                                 jlong interpreter_ptr,
                                                 jlong script_ptr,
                                                 jobject completion_callback) {
    (void) clazz;
    RubyInterpreter* interpreter = (RubyInterpreter*)interpreter_ptr;
    RubyScript* script = (RubyScript*)script_ptr;

    if (!interpreter || !script) {
        // If callback exists, call it with error result
        if (completion_callback) {
            int context_result;
            CompletionCallbackContext* error_context = create_completion_context(env, completion_callback, &context_result);
            if (error_context) {
                // Call callback with error result (1) immediately
                (*env)->CallVoidMethod(env, error_context->callback_obj,
                                      error_context->invoke_method_id, (jint)1);
                destroy_completion_context(error_context);
            } else {
                fprintf(stderr, "Error while creating the completion context %d\n", context_result);
            }
        }
        return;
    }

    // Clean up any existing completion context
    if (g_completion_context) {
        destroy_completion_context(g_completion_context);
        g_completion_context = NULL;
    }

    RubyCompletionTask c_completion_callback = NULL;

    // Create completion callback context if callback is provided
    if (completion_callback) {
        int context_result;
        g_completion_context = create_completion_context(env, completion_callback, &context_result);
        if (g_completion_context) {
            c_completion_callback = jni_completion_callback;
        } else {
            fprintf(stderr, "Error while creating the completion context %d\n", context_result);
            // Failed to create context, call callback with error immediately
            jclass callback_class = (*env)->GetObjectClass(env, completion_callback);
            jmethodID complete_method = (*env)->GetMethodID(env, callback_class, "complete", "(I)V");
            if (complete_method) {
                (*env)->CallVoidMethod(env, completion_callback, complete_method, (jint)1);
            }
            return;
        }
    }

    // Enqueue the script with the completion callback
    const int interpreter_script_result = ruby_interpreter_enqueue(interpreter, script, c_completion_callback);
    if (interpreter_script_result != 0) {
        fprintf(stderr, "Error while interpreting script: %d\n", interpreter_script_result);
    }
}

JNIEXPORT jint JNICALL
Java_com_scorbutics_rubyvm_RubyVMNative_updateEnvLocations(JNIEnv *env, jclass clazz, jstring current_directory, jstring extra_arg) {
    (void) clazz;
    char* c_current_directory = jstring_to_cstring(env, current_directory);
    char* c_extra_arg = jstring_to_cstring(env, extra_arg);

    const int result = env_update_locations(c_current_directory, c_extra_arg);

    free(c_extra_arg);
    free(c_current_directory);
    return result;
}
