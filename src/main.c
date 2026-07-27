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
    
    return 0;
}