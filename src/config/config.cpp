#include <fstream>
#include <mutex>
#include <nlohmann/json.hpp>

#include "config.h"

using std::string;
using nlohmann::json;


std::mutex g_cfg_mtx;
config_t g_cfg;


config_t get_config() {
    std::lock_guard<std::mutex> lock(g_cfg_mtx);
    return g_cfg;
}

int check_config_file() {
    return 0;
}

int load_config() {
    std::ifstream config_map(CFG_FILE_PATH);
    json data;
    try {
        config_map >> data;
    } catch (...) {
        fprintf(stderr, "%s %u wrong json format\n", __FUNCTION__, __LINE__);
        return 1;
    }

    if (data.empty()) {
         return 1;
    }
    
    if(!data.contains("grpc")) {
        return 1;
    }

    config_t cfg;

    json &cfg_grpc = data["grpc"];

    string target_addr = cfg_grpc["target_addr"].get<string>();
    size_t target_addr_len = target_addr.length();
    if (target_addr_len >= CFG_GRPC_ADDR_LEN) {
        return 1;
    }
    memset(cfg.grpc.target_addr, 0, sizeof(cfg.grpc.target_addr));
    strncpy(cfg.grpc.target_addr, target_addr.c_str(), target_addr_len);
    cfg.grpc.target_addr[target_addr_len] = '\0';

    string srv_addr = cfg_grpc["srv_addr"].get<string>();
    size_t srv_addr_len = srv_addr.length();
     if (srv_addr_len >= CFG_GRPC_ADDR_LEN) {
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

void watch_config_file(void *arg) {
    load_config();
}