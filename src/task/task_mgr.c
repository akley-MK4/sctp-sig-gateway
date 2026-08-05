#include <stdio.h>

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
        printf("Task created successfully with ID: %d\n", createTaskId);
    } else {
        printf("Failed to create task, error code: %d\n", errCode);
    }
}

int create_task(const char* task_name) {
    // just for testing
    if (grpc_create_task_async(task_name, task_creation_callback) == 0) {
        printf("Task created successfully\n");
    } else {
        printf("Failed to create task\n");
    }

    return 0; // Return success
}