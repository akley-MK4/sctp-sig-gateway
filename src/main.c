#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "event_loop.h"      /* Singleton event loop      */
#include "signal_handler.h"  /* Signal registration        */
#include "file_watcher.h"
#include "config.h"
#include "grpc_export.h"

static void on_sigint(int signum, void *arg) {
    struct event_base *eb = (struct event_base *)arg;
    printf("received SIGINT (%d), shutting down gracefully...\n", signum);

    file_watcher_shutdown();
    signal_handler_shutdown();
    event_base_loopbreak(eb);   /* causes event_base_dispatch() to return */
}

int main() {
    // Initialize event loop
    if (event_loop_init() != 0) {
        fprintf(stderr, "Failed to initialize event loop\n");
        return 1;
    }

    // Register signal handlers
    register_signal_handler(event_loop_get_base(), SIGINT,  on_sigint, NULL);
    register_signal_handler(event_loop_get_base(), SIGTERM, on_sigint, NULL);
    
    // Config
    if (load_config() != 0) {
        fprintf(stderr, "Failed to load the config file\n");
        return 1;
    }

    int err_code = initialize_grpc_mgr();
    if (err_code != 0) {
        return 1;
    }

    err_code = start_grpc_mgr();
    if (err_code != 0) {
        return 1;
    }

    // watch config file
    register_file_watcher(event_loop_get_base(), CFG_FILE_PATH, on_config_file_updated, NULL);
    

    // just for testing
    char *error_message = NULL;
    if (grpc_create_task("MyTask", error_message) == 0) {
        printf("Task created successfully\n");
    } else {
        printf("Failed to create task\n");
    }

    // Block main thread: enter event loop
    // Runs until SIGINT/SIGTERM triggers loopbreak
    if (event_loop_run() != 0) {
        goto cleanup;
    }

cleanup:
    event_loop_cleanup();

    return 0;
}