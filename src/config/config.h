#ifndef CONFIG_H_
#define CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define CFG_GRPC_ADDR_LEN 32

typedef struct config_grpc_s {
    char target_addr[CFG_GRPC_ADDR_LEN];
    char srv_addr[CFG_GRPC_ADDR_LEN];

} config_grpc_t;

typedef struct config_s {
    config_grpc_t grpc;

} config_t;


//config_t g_config;

//int check_config_file();
int load_config();

#ifdef __cplusplus
}
#endif

#endif