#ifndef CONFIG_H_
#define CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_CFG_GRPC_ADDR_LEN 128
#define DEFAULT_CFG_FILE_PATH "/etc/config/config.json"

typedef struct config_grpc_s {
    char target_addr[MAX_CFG_GRPC_ADDR_LEN];
    char srv_addr[MAX_CFG_GRPC_ADDR_LEN];

} config_grpc_t;

typedef struct config_s {
    config_grpc_t grpc;

} config_t;

//int check_config_file();
void set_config_path(const char* config_path);
const char* get_config_path();
int load_config();
config_t get_config();
void on_config_file_updated(void *arg);

#ifdef __cplusplus
}
#endif

#endif