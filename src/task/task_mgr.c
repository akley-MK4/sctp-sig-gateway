#include <stdio.h>

#include "logger.h"
#include "task_mgr.h"
#include "grpc_export.h"

static task_mgr_t g_task_mgr;

int initialize_task_mgr() {
    
    return 0;
}

int start_task_mgr() {
    // Implementation of starting the task manager
    return 0; // Return success
}

static void task_creation_callback(int errCode, int createTaskId) {
    if (errCode == 0) {
        log_info("Task created successfully with ID: %d", createTaskId);
    } else {
        log_error("Failed to create task, error code: %d", errCode);
    }
}

int create_task(const char* task_name) {
    // just for testing
    if (grpc_create_task_async(task_name, task_creation_callback) != 0) {
        log_error("Failed to create task");
    }

    return 0; // Return success
}