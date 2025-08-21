#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <malloc.h>
#include <errno.h>

#include "logging.h"

#define MAX_LOGFILE_SIZE 100 * 1024

int g_logging_thread_continue = 1;
pthread_t g_logging_thread = 0;
static int pfd[2];
static char* log_tag = NULL;
static int logFd = -1;
static logging_native_logging_func_t loggingFunction = NULL;

enum {
	LOG_UNKNOWN = 0,
	LOG_DEFAULT,
	LOG_VERBOSE,
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARN,
	LOG_ERROR,
	LOG_FATAL,
	LOG_SILENT
};

static void LogNative(int prio, const char* tag, const char* text) {
	if (loggingFunction != NULL) {
		loggingFunction(prio, tag, text);
	}
}

static void WriteFullLogLine(const char* line) {
	LogNative(LOG_INFO, log_tag == NULL ? "UNKNOWN" : log_tag, line);
	if (logFd > 0) {
        const size_t lineSize = strlen(line);
		write(logFd, line, lineSize);
	}
}

static void SendBufferToOutputAsLine(char *buffer, size_t *size) {
	if (*size > 0) {
		buffer[*size] = '\0';
		WriteFullLogLine(buffer);
		*size = 0;
	}
}

static int ResizeGlobalLogBufferIfNeeded(char** globalBuffer, size_t* globalBufferCapacity, size_t newSize) {
	if (*globalBufferCapacity <= newSize) {
		*globalBufferCapacity = newSize * 1.5;
		char* newGlobalBuffer = realloc(*globalBuffer, *globalBufferCapacity * sizeof(*globalBuffer));
		if (*globalBuffer == NULL && *globalBufferCapacity != 0) {
			free(*globalBuffer);
			*globalBufferCapacity = 0;
			return 1;
		}
		*globalBuffer = newGlobalBuffer;
	}
	return 0;
}

static int AppendLogToBuffer(const char* buf, char** globalBuffer, size_t* globalBufferCapacity, size_t* globalBufferSize, size_t stringSizeToAppend) {
	if (ResizeGlobalLogBufferIfNeeded(globalBuffer, globalBufferCapacity, *globalBufferSize + stringSizeToAppend + 1) != 0) {
		LogNative(LOG_ERROR, log_tag == NULL ? "UNKNOWN" : log_tag, "Internal memory error, aborting thread");
		return 1;
	}

	memcpy(*globalBuffer + *globalBufferSize, buf, stringSizeToAppend);
	*globalBufferSize += stringSizeToAppend;
	return 0;
}

static void* loggingFunctionThread(void* unused) {
	(void) unused;
	ssize_t readSize;
	char buf[128];

	char* globalBuffer = (char*) malloc(sizeof(buf));
	if (globalBuffer == NULL) {
		LogNative(LOG_ERROR, log_tag == NULL ? "UNKNOWN" : log_tag, "Internal memory error, aborting logging thread");
		return NULL;
	}
	size_t globalBufferSize = 0;
	size_t globalBufferCapacity = sizeof(buf) / sizeof(*buf);

	while (g_logging_thread_continue && ((readSize = read(pfd[0], buf, sizeof buf)) > 0)) {
		size_t remainingLineStartBufIndex = 0;
		for (ssize_t index = 0; index < readSize; index++) {
			if (buf[index] == '\n') {
				/* When a line break, we append everything to the globalBuffer and then log it */
				AppendLogToBuffer(buf + remainingLineStartBufIndex, &globalBuffer, &globalBufferCapacity,
								  &globalBufferSize, index - remainingLineStartBufIndex + 1);
				SendBufferToOutputAsLine(globalBuffer, &globalBufferSize);
				remainingLineStartBufIndex = index + 1;
			}
		}

		/* Append the remaining and do nothing else */
		if (readSize - remainingLineStartBufIndex > 0) {
			AppendLogToBuffer(buf + remainingLineStartBufIndex, &globalBuffer, &globalBufferCapacity,
							  &globalBufferSize, readSize - remainingLineStartBufIndex);
		}
	}

	if (readSize == -1) {
		char errorMessage[512];
		sprintf(errorMessage, "Unable to read from log file/pipe: %s", strerror(errno));
		LogNative(LOG_ERROR, log_tag == NULL ? "UNKNOWN" : log_tag, errorMessage);
	} else {
		SendBufferToOutputAsLine(globalBuffer, &globalBufferSize);
	}
	WriteFullLogLine("----------------------------");
	LogNative(LOG_DEBUG, log_tag == NULL ? "UNKNOWN" : log_tag, "Logging thread ended");

    free(globalBuffer);
    free(log_tag);
	return NULL;
}

void LoggingSetNativeLoggingFunction(logging_native_logging_func_t func) {
	loggingFunction = func;
}

int LoggingThreadRun(const char* appname, int extraLogFd) {
	setvbuf(stdout, 0, _IOLBF, 0); // make stdout line-buffered
	setvbuf(stderr, 0, _IONBF, 0); // make stderr unbuffered

	log_tag = strdup(appname);
	logFd = extraLogFd;

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, pfd) == -1) {
        LogNative(LOG_ERROR, "Logging", "socketpair() failed");
        return -3;
    }

	dup2(pfd[1], STDERR_FILENO);
	dup2(pfd[1], STDOUT_FILENO);

	if (pthread_create(&g_logging_thread, 0, loggingFunctionThread, 0) != 0) {
		LogNative(LOG_WARN, log_tag == NULL ? "UNKNOWN" : log_tag,
											"Cannot spawn logging thread : logging from stdout / stderr won't show");
		return -1;
	}
	LogNative(LOG_DEBUG, log_tag == NULL ? "UNKNOWN" : log_tag, "Logging thread started");
	return 0;
}
