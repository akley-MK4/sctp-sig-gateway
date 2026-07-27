#pragma once
#ifdef __cplusplus
extern "C" {
#endif

int wrap_initialize_grpc_mgr(const char* target_addr, const char* srv_addr);
int wrap_start_grpc_mgr();
int create_task(const char* task_name, char* error_message);

#ifdef __cplusplus
}
#endif