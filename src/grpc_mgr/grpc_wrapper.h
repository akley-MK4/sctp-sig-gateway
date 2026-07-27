#pragma once
#ifdef __cplusplus
extern "C" {
#endif

int wrap_initialize_grpc_mgr(const char* target_addr, const char* srv_addr);
int wrap_start_grpc_mgr();

#ifdef __cplusplus
}
#endif