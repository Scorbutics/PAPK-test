#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "constants.h"

#include "ruby-script-location.h"
#include "ruby-script.h"
#include "ruby-vm.h"
#include "exec-main-vm.h"

static int ruby_vm_native_exec(const char* script_content,
                        int logs_fd,
                        int commands_fd,
                        const char* ruby_base_directory,
                        const char* native_libs_location) {
    return ExecMainRubyVM(script_content, logs_fd, commands_fd,
        ruby_base_directory,
        native_libs_location);
}

RubyVM* ruby_vm_create(const char* application_path, RubyScript* main_script, LogListener listener) {
    if (!application_path || !main_script) return NULL;

    RubyVM* vm = malloc(sizeof(RubyVM));
    if (!vm) return NULL;

    vm->application_path = strdup(application_path);
    vm->main_script = main_script;
    vm->log_listener = listener;
    vm->vm_started = 0;

    return vm;
}

void ruby_vm_destroy(RubyVM* vm) {
    if (!vm) return;

    // Close communication channels
    close_comm_channel(&vm->logs_channel);
    close_comm_channel(&vm->commands_channel);

    free(vm->application_path);
    free(vm);
}

int ruby_vm_start(RubyVM* vm, const char* ruby_base_directory, const char* native_libs_location) {
    // Already started, no need to change anything
    if (vm->vm_started) return 1;

    // Create socket pairs for communication
    if (create_comm_channel(&vm->logs_channel) != 0 ||
        create_comm_channel(&vm->commands_channel) != 0) {
        return -1; // Failed to create channels
    }

    // Create thread arguments
    RubyVMStartArgs* transferedMemoryArgs = malloc(sizeof(RubyVMStartArgs));
    transferedMemoryArgs->vm = vm;
    transferedMemoryArgs->ruby_base_directory = strdup(ruby_base_directory);
    transferedMemoryArgs->native_libs_location = strdup(native_libs_location);

    // Start main thread
    // "transferedMemoryArgs" is consumed and freed by the main thread
    pthread_create(&vm->main_thread, NULL, ruby_vm_main_thread_func, transferedMemoryArgs);

    // Start log reader thread
    pthread_create(&vm->log_reader_thread, NULL, ruby_vm_log_reader_thread_func, vm);

    vm->vm_started = 1;
    return 0;
}

void ruby_vm_enqueue(RubyVM* vm, RubyScript* script, RubyCompletionTask on_complete) {
    if (!vm || !script) return;

    RubyScriptEnqueueArgs* transferedMemoryArgs = malloc(sizeof(RubyScriptEnqueueArgs));
    transferedMemoryArgs->vm = vm;
    transferedMemoryArgs->script = script;
    transferedMemoryArgs->on_complete = on_complete;

    pthread_t script_thread;
    // "transferedMemoryArgs" is consumed and freed by the thread
    pthread_create(&script_thread, NULL, ruby_vm_script_thread_func, transferedMemoryArgs);
    pthread_detach(script_thread);

    const char* content = ruby_script_get_content(script);
    write_to_comm_channel(&vm->commands_channel, content, strlen(content));
}

// Thread functions
void* ruby_vm_main_thread_func(void* arg) {
    RubyVMStartArgs* args = (RubyVMStartArgs*)arg;
    RubyVM* vm = args->vm;

    ruby_vm_native_exec(
        ruby_script_get_content(vm->main_script),
        vm->logs_channel.write_fd,
        vm->commands_channel.write_fd, // Usage for both read and write
        args->ruby_base_directory,
        args->native_libs_location
    );

    free(args->native_libs_location);
    free(args->ruby_base_directory);
    free(args);
    return NULL;
}

void* ruby_vm_log_reader_thread_func(void* arg) {
    RubyVM* vm = (RubyVM*)arg;

   // Read from socket channel
    char line[1024];
    int bytes_read;

    while ((bytes_read = read_from_comm_channel(&vm->logs_channel, line, sizeof(line))) > 0) {
        // Remove newline if present
        char* newline = strchr(line, '\n');
        if (newline) *newline = '\0';

        if (vm->log_listener.accept) {
            vm->log_listener.accept(line);
        }
    }
    return NULL;
}

void* ruby_vm_script_thread_func(void* arg) {
    RubyScriptEnqueueArgs* args = (RubyScriptEnqueueArgs*)arg;
    RubyVM* vm = args->vm;
    RubyCompletionTask on_complete = args->on_complete;
    char result_line[256];
    int result = 1; // Default to error

    // Read from socket channel
    if (read_from_comm_channel(&vm->commands_channel, result_line, sizeof(result_line)) > 0) {
        result = (strncmp(result_line, "0", 1) == 0) ? 0 : 1;
    }
    if (on_complete) on_complete(result);

    free(args);
    return NULL;
}
