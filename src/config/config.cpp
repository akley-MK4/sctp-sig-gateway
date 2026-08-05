#include <fstream>
#include <mutex>
#include <nlohmann/json.hpp>

#include "config.h"

using std::string;
using nlohmann::json;


std::mutex g_cfg_mtx;
static config_t g_cfg;
static string g_config_path = DEFAULT_CFG_FILE_PATH;


config_t get_config() {
    std::lock_guard<std::mutex> lock(g_cfg_mtx);
    return g_cfg;
}

int check_config_file() {
    return 0;
}

void set_config_path(const char* config_path) {
    std::lock_guard<std::mutex> lock(g_cfg_mtx);
    if (strlen(config_path) == 0) {
        g_config_path = DEFAULT_CFG_FILE_PATH;
    } else {
        g_config_path = string(config_path);
    }
    printf("%s %u, Set the configuration path to %s\n", __FUNCTION__, __LINE__, g_config_path.c_str());
}

const char* get_config_path() {
    std::lock_guard<std::mutex> lock(g_cfg_mtx);
    return g_config_path.c_str();
}

int load_config() {
    std::ifstream config_map(g_config_path);
    json data;
    try {
        config_map >> data;
    } catch (...) {
        fprintf(stderr, "%s %u wrong json format\n", __FUNCTION__, __LINE__);
        return 1;
    }

    if (data.empty()) {
        fprintf(stderr, "%s %u data is empty\n", __FUNCTION__, __LINE__);
         return 1;
    }
    
    if(!data.contains("grpc")) {
        fprintf(stderr, "%s %u grpc config not found\n", __FUNCTION__, __LINE__);
        return 1;
    }

    config_t cfg;

    json &cfg_grpc = data["grpc"];

    string target_addr = cfg_grpc["target_addr"].get<string>();
    size_t target_addr_len = target_addr.length();
    if (target_addr_len >= MAX_CFG_GRPC_ADDR_LEN) {
        fprintf(stderr, "%s %u target_addr is too long\n", __FUNCTION__, __LINE__);
        return 1;
    }
    memset(cfg.grpc.target_addr, 0, sizeof(cfg.grpc.target_addr));
    strncpy(cfg.grpc.target_addr, target_addr.c_str(), target_addr_len);
    cfg.grpc.target_addr[target_addr_len] = '\0';

    string srv_addr = cfg_grpc["srv_addr"].get<string>();
    size_t srv_addr_len = srv_addr.length();
     if (srv_addr_len >= MAX_CFG_GRPC_ADDR_LEN) {
        fprintf(stderr, "%s %u srv_addr is too long\n", __FUNCTION__, __LINE__);
        return 1;
    }
    memset(cfg.grpc.srv_addr, 0, sizeof(cfg.grpc.srv_addr));
    strncpy(cfg.grpc.srv_addr, srv_addr.c_str(), srv_addr_len);
    cfg.grpc.srv_addr[srv_addr_len] = '\0';

    {
        std::lock_guard<std::mutex> lock(g_cfg_mtx);
        g_cfg = cfg;
    }

    return 0;
}

void on_config_file_updated(void *arg) {
    (void)arg;
    load_config();
}