#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "signal_wait.h"
#include "grpc_wrapper.h"


int main() {
    const char* target_addr = "localhost:50051";
    const char* srv_addr = "0.0.0.0:50051";

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

    signal_wait_init();
    signal_wait_forever();
    
    return 0;
}