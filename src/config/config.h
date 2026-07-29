#ifndef CONFIG_H_
#define CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define CFG_GRPC_ADDR_LEN 32
#define CFG_FILE_PATH "/etc/config/config.json"

typedef struct config_grpc_s {
    char target_addr[CFG_GRPC_ADDR_LEN];
    char srv_addr[CFG_GRPC_ADDR_LEN];

} config_grpc_t;

typedef struct config_s {
    config_grpc_t grpc;

} config_t;

//int check_config_file();
int load_config();
config_t get_config();
void watch_config_file(void *arg);

#ifdef __cplusplus
}
#endif

#endif