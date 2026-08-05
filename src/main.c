#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "event_loop.h"      /* Singleton event loop      */
#include "signal_handler.h"  /* Signal registration        */
#include "file_watcher.h"
#include "config.h"
#include "grpc_export.h"
#include "task_mgr.h"

static void on_sigint(int signum, void *arg) {
    struct event_base *eb = (struct event_base *)arg;
    printf("received SIGINT (%d), shutting down gracefully...\n", signum);

    file_watcher_shutdown();
    signal_handler_shutdown();
    event_base_loopbreak(eb);   /* causes event_base_dispatch() to return */
}

int main(int argc, char *argv[]) {
    printf("%s %u, argc: %d, app: %s\n", __FUNCTION__, __LINE__, argc, argv[0]);
    if (argc < 3) {
        fprintf(stderr, "Usage: %s [config_file_path] [server|client]\n", argv[0]);
        return 1;
    }

    const char* config_path = argv[1];

    int is_server = 0;
    if (strcmp(argv[2], "server") == 0) {
        is_server = 1;
        printf("%s %u, The service type has been set to server\n", __FUNCTION__, __LINE__);
    } else if (strcmp(argv[2], "client") == 0) {
        is_server = 0;
        printf("%s %u, The service type has been set to client\n", __FUNCTION__, __LINE__);
    } else {
        fprintf(stderr, "Invalid argument: %s. Use 'server' or 'client'.\n", argv[2]);
        return 1;
    }

    // Initialize event loop
    if (event_loop_init() != 0) {
        fprintf(stderr, "Failed to initialize event loop\n");
        return 1;
    }

    // Register signal handlers
    register_signal_handler(event_loop_get_base(), SIGINT,  on_sigint, NULL);
    register_signal_handler(event_loop_get_base(), SIGTERM, on_sigint, NULL);

    // Config
    set_config_path(config_path);
    if (load_config() != 0) {
        fprintf(stderr, "Failed to load the config file\n");
        return 1;
    }

    int err_code = initialize_grpc_mgr();
    if (err_code != 0) {
        return 1;
    }

    err_code = initialize_task_mgr();
    if (err_code != 0) {
        return 1;
    }


    err_code = start_grpc_mgr();
    if (err_code != 0) {
        return 1;
    }

    err_code = start_task_mgr();
    if (err_code != 0) {
        return 1;
    }

    // watch config file
    register_file_watcher(event_loop_get_base(), get_config_path(), on_config_file_updated, NULL);
    
    // just for testing
    if (is_server == 1) {
        sleep(5);
        create_task("test_task_1");
    }

    // Block main thread: enter event loop
    // Runs until SIGINT/SIGTERM triggers loopbreak
    if (event_loop_run() != 0) {
        goto cleanup;
    }

    stop_grpc_mgr();

    
cleanup:
    event_loop_cleanup();

    return 0;
}