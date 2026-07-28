#include <fstream>
#include <nlohmann/json.hpp>

#include "config.h"

using std::string;
using nlohmann::json;

#define CFG_FILE_PATH "/etc/config/config.json"

int check_config_file() {
    return 0;
}

int load_config()
{
    std::ifstream config_map(CFG_FILE_PATH);
    json data;
    try {
        config_map >> data;
    }catch (...) {
        fprintf(stderr, "%s %u wrong json format\n", __FUNCTION__, __LINE__);
        return 1;
    }

    if (data.empty()) {
         return 1;
    }
    
    if(!data.contains("grpc")) {
        return 1;
    }

    config_t g_config;
    
    json &cfg_grpc = data["grpc"];

    string target_addr = cfg_grpc["target_addr"].get<string>();
    size_t target_addr_len = target_addr.length();
    memset(g_config.grpc.target_addr, 0, sizeof(g_config.grpc.target_addr));
    strncpy(g_config.grpc.target_addr, target_addr.c_str(), target_addr_len);
    g_config.grpc.target_addr[target_addr_len] = '\0';

    string srv_addr = cfg_grpc["srv_addr"].get<string>();
    size_t srv_addr_len = srv_addr.length();
    memset(g_config.grpc.srv_addr, 0, sizeof(g_config.grpc.srv_addr));
    strncpy(g_config.grpc.srv_addr, srv_addr.c_str(), srv_addr_len);
    g_config.grpc.srv_addr[srv_addr_len] = '\0';

    return 0;
}