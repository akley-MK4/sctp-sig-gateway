#pragma once
#ifdef __cplusplus
extern "C" {
#endif

void* grpc_create_channel(const char* target);
int grpc_create_task(void* grpc_channel, const char* task_name, char* error_message);
void grpc_destroy_channel(void* channel);

#ifdef __cplusplus
}
#endif