#include "config.h"
#include "logger.h"
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
    initializeChannel();

    for (int i = 0; i < MAX_NUM_ASYNC_CLIENTS; ++i) {
        // All AsyncClients share the same underlying HTTP/2 connection
        auto client = std::make_unique<AsyncClient>(i, channel_);
        clients_.push_back(std::move(client));
    }
    log_info("[GrpcMgr] initialize() created %zu async clients", clients_.size());

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


void GrpcMgr::initializeChannel() {
    grpc::ChannelArguments args;
    args.SetInt(GRPC_ARG_KEEPALIVE_TIME_MS, 60 * 1000);
    args.SetInt(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, 10 * 1000);
    args.SetInt(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS, 1);
    args.SetInt(GRPC_ARG_HTTP2_MAX_PINGS_WITHOUT_DATA, 0);

    channel_ = grpc::CreateCustomChannel(target_addr_, grpc::InsecureChannelCredentials(), args);
    log_info("Initialized the channel, target_addr_: %s", target_addr_.c_str());
}

std::shared_ptr<ChannelInterface> GrpcMgr::getChannel() {
    return channel_;
}

bool GrpcMgr::IsStarted() {
    return started_;
}

void GrpcMgr::startServer() {
    grpc::ServerBuilder builder;
    // keep alive
    builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIME_MS, 1000 * 60);
    builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, 1000 * 10);
    builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS, 1);
    builder.AddChannelArgument(GRPC_ARG_HTTP2_MAX_PINGS_WITHOUT_DATA, 0);
    builder.AddChannelArgument(GRPC_ARG_HTTP2_MIN_RECV_PING_INTERVAL_WITHOUT_DATA_MS, 1000 * 30);
    builder.AddChannelArgument(GRPC_ARG_HTTP2_MAX_PING_STRIKES, 3);
    // connection age
    builder.AddChannelArgument(GRPC_ARG_MAX_CONNECTION_AGE_MS, 1000 * 60 * 60 * 12);
    builder.AddChannelArgument(GRPC_ARG_MAX_CONNECTION_AGE_GRACE_MS, 1000 * 10);
    
    builder.AddListeningPort(srv_addr_, grpc::InsecureServerCredentials());
    builder.RegisterService(&service_);

    server_ = builder.BuildAndStart();
    server_thread_ = std::thread(&GrpcMgr::serverThreadFunc, this);
}

void GrpcMgr::startAsyncClients() {
    log_info("[GrpcMgr] Starting %zu async clients", clients_.size());
    for (auto& client : clients_) {
        bool ok = client->Start();
        log_info("[GrpcMgr] Client id=%d Start() returned=%d", client->Id(), ok);
    }
}

void GrpcMgr::stopAsyncClients() {
    for (auto& client : clients_) {
        client->Stop();
    }
}

void GrpcMgr::serverThreadFunc() {
    log_info("[GrpcMgr] Listening on %s", srv_addr_.c_str());
    log_info("[GrpcMgr] Server thread started, waiting for requests...");
    server_->Wait();
    log_info("[GrpcMgr] Server thread exited");
}

AsyncClient* GrpcMgr::PickClient() {
    if (clients_.empty()) return nullptr;
    size_t idx = pick_counter_.fetch_add(1) % clients_.size();
    return clients_[idx].get();
}
