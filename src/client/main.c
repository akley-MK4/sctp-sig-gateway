#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "grpc_wrapper.h"

int main(int argc, char *argv[]) {
    const char *target = "localhost:50051";
    if (argc > 1) {
        target = argv[1];
    }

    printf("Connecting to %s...\n", target);

    // Create a gRPC Channel
    void *channel = grpc_create_channel(target);
    if (!channel) {
        fprintf(stderr, "Failed to create channel\n");
        return 1;
    }

    for (int i=0; i<10; i++) {
        sleep(1);

        // Call grpc_create_task
        char *error_message = NULL;
        if (grpc_create_task(channel, "MyTask", error_message) == 0) {
            printf("Task created successfully\n");
        } else {
            fprintf(stderr, "Failed to create task\n");
        }

    }

    grpc_destroy_channel(channel);

    return 0;
}