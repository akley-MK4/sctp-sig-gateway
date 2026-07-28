#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "event_loop.h"      /* Singleton event loop      */
#include "signal_handler.h"  /* Signal registration        */
#include "grpc_wrapper.h"


int main() {
    const char* target_addr = "localhost:50051";
    const char* srv_addr = "0.0.0.0:50051";

    struct signal_handler *sig_h = NULL;
    int ret = 1;  /* default exit code: failure */

    // Initialize event loop
    if (event_loop_init() != 0) {
        fprintf(stderr, "Failed to initialize event loop\n");
        return 1;
    }

    // Register signal handler on the same base
    sig_h = signal_handler_new(event_loop_get_base());
    if (sig_h == NULL || signal_handler_start(sig_h) != 0) {
        fprintf(stderr, "Failed to register signal handler\n");
        goto cleanup;
    }

    int err_code = wrap_initialize_grpc_mgr(target_addr, srv_addr);
    if (err_code != 0) {
        return 1;
    }

    err_code = wrap_start_grpc_mgr();
    if (err_code != 0) {
        return 1;
    }

    // just for testing
    char *error_message = NULL;
    if (create_task("MyTask", error_message) == 0) {
        printf("Task created successfully\n");
    } else {
        printf("Failed to create task\n");
    }

    // Block main thread: enter event loop
    // Runs until SIGINT/SIGTERM triggers loopbreak
    if (event_loop_run() != 0) {
        goto cleanup;
    }

    ret = 0;  /* success */

cleanup:
    /* ---------- 5. Cleanup ---------- */
    if (sig_h != NULL) {
        signal_handler_free(sig_h);
    }
    event_loop_cleanup();

    return ret;
}