#ifndef LOG_LISTENER_H
#define LOG_LISTENER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*LogAcceptFunc)(const char* lineMessage);
typedef void (*LogErrorFunc)(const char* errorMessage);

typedef struct LogListener {
    void* context;
    LogAcceptFunc accept;
    LogErrorFunc on_log_error;
} LogListener;

#ifdef __cplusplus
}
#endif

#endif