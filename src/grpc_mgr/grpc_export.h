#ifndef GRPC_EXPORT_H_
#define GRPC_EXPORT_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*grpc_create_task_callback)(int errCode, int createTaskId);

int initialize_grpc_mgr();
int stop_grpc_mgr();
int start_grpc_mgr();
int grpc_create_task(const char* task_name, char* error_message);
int grpc_create_task_async(const char* task_name, grpc_create_task_callback cb);
int grpc_create_task_async_with_cbapi(const char* task_name, grpc_create_task_callback cb);

#ifdef __cplusplus
}
#endif

#endif