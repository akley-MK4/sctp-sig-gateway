#include <stdio.h>

#include "logger.h"
#include "task_mgr.h"
#include "grpc_export.h"

static task_mgr_t g_task_mgr;

static void task_creation_callback(int errCode, int createTaskId) {
    if (errCode == 0) {
        log_info("Task created successfully with ID: %d", createTaskId);
    } else {
        log_error("Failed to create task, error code: %d", errCode);
    }
}

static void on_timer(evutil_socket_t fd, short what, void *arg) {
    if (g_task_mgr.running == 0) {
        return;
    }

    // just for testing
    if (grpc_create_task_async("task-test1", task_creation_callback) != 0) {
        log_error("Failed to create task");
    }

    return;
}

int initialize_task_mgr(int is_server) {
    g_task_mgr.is_server = is_server;
    g_task_mgr.running = 0;

    struct event *timer = register_periodic_timer(10, 0, on_timer, NULL);
    if (timer == NULL) {
        log_error("Failed to register heartbeat timer");
        return 1;
    }
    
    g_task_mgr.timer = timer;
    return 0;
}

int start_task_mgr() {
    g_task_mgr.running = 1;
    return 0; // Return success
}
