#ifndef LOGGING_H
#define LOGGING_H

#ifdef __cplusplus
extern "C" {
#endif
#include <pthread.h>
typedef int (*logging_native_logging_func_t) (int, const char*, const char*);

int LoggingThreadRun(const char* appname, int extraLogFd);
void LoggingSetNativeLoggingFunction(logging_native_logging_func_t func);

extern int g_logging_thread_continue;
extern pthread_t g_logging_thread;
#ifdef __cplusplus
}
#endif

#endif //LOGGING_H
