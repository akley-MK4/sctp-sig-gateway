#ifndef GRPC_EXPORT_H_
#define GRPC_EXPORT_H_

#ifdef __cplusplus
extern "C" {
#endif

int wrap_initialize_grpc_mgr();
int wrap_start_grpc_mgr();
int create_task(const char* task_name, char* error_message);

#ifdef __cplusplus
}
#endif

#endif