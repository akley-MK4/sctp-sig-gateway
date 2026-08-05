#include "config.h"
#include "grpc_mgr.h"
#include "grpc_svc_impl.h"

using std::string;

#define MAX_NUM_ASYNC_CLIENTS 1

GrpcMgr::GrpcMgr() 
  : initialized_(false), 
    started_(false) 
{

}

GrpcMgr::~GrpcMgr() {
    if (!initialized_) {
        return;
    }

    channel_.reset();
}

GrpcMgr& GrpcMgr::GetInstance() {
    static GrpcMgr instance;
    return instance;
}

int GrpcMgr::initialize() {
    config_t cfg = get_config();
    target_addr_ = string(cfg.grpc.target_addr);
    srv_addr_ = string(cfg.grpc.srv_addr);
    channel_ = grpc::CreateChannel(target_addr_, grpc::InsecureChannelCredentials());
    printf("%s %u created the channel, target_addr_: %s, srv_addr_: %s\n", __FUNCTION__, __LINE__, target_addr_.c_str(), srv_addr_.c_str());

    for (int i = 0; i < MAX_NUM_ASYNC_CLIENTS; ++i) {
        // All AsyncClients share the same underlying HTTP/2 connection
        auto client = std::make_unique<AsyncClient>(i, channel_);
        clients_.push_back(std::move(client));
    }

    initialized_ = true;
    return 0;
}

int GrpcMgr::start() {
    if (!initialized_) {
        return 1;
    }
    if (started_) {
        return 1;
    }
    started_ = true;

    startServer();
    startAsyncClients();

    return 0;
}

int GrpcMgr::stop() {
    stopAsyncClients();
    return 0;
}

std::shared_ptr<ChannelInterface> GrpcMgr::getChannel() {
    return channel_;
}

bool GrpcMgr::IsStarted() {
    return started_;
}

void GrpcMgr::startServer() {
    grpc::ServerBuilder builder;
    builder.AddListeningPort(srv_addr_, grpc::InsecureServerCredentials());
    builder.RegisterService(&service_);

    server_ = builder.BuildAndStart();
    std::cout << "[Server] Listening on " << srv_addr_ << std::endl;
    server_thread_ = std::thread(&GrpcMgr::serverThreadFunc, this);
}

void GrpcMgr::startAsyncClients() {
    for (auto& client : clients_) {
        client->Start();
    }
}

void GrpcMgr::stopAsyncClients() {
    for (auto& client : clients_) {
        client->Stop();
    }
}

void GrpcMgr::serverThreadFunc() {
    std::cout << "[GrpcMgr] Server thread started, waiting for requests..." << std::endl;
    server_->Wait();
    std::cout << "[GrpcMgr] Server thread exited" << std::endl;
}

AsyncClient* GrpcMgr::PickClient() {
    if (clients_.empty()) return nullptr;
    size_t idx = pick_counter_.fetch_add(1) % clients_.size();
    return clients_[idx].get();
}
