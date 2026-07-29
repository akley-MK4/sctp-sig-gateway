#ifndef GRPC_EXPORT_H_
#define GRPC_EXPORT_H_

#ifdef __cplusplus
extern "C" {
#endif

int initialize_grpc_mgr();
int start_grpc_mgr();
int grpc_create_task(const char* task_name, char* error_message);

#ifdef __cplusplus
}
#endif

#endif