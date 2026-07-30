#ifndef GRPC_MGR_H_
#define GRPC_MGR_H_

#include <thread>
#include <grpcpp/create_channel.h>
#include "grpc_svc_impl.h"
#include "grpc_async_client.h"

using std::string;
using grpc::ChannelInterface;

class GrpcMgr {
public:
    static GrpcMgr& GetInstance();
    int initialize();
    int Start();
    int CreateTask(const char* task_name, char* error_message);
    bool IsStarted();
    std::shared_ptr<ChannelInterface> getChannel();
    AsyncClient* PickClient();

private:
    GrpcMgr();
    ~GrpcMgr();
    void serverThreadFunc();
    void startServer();
    void startAsyncClients();

    //std::atomic<bool> initialized_{false};
    bool initialized_;
    bool started_;
    string target_addr_;
    string srv_addr_;
    std::shared_ptr<ChannelInterface> channel_;
    // client
    std::atomic<uint64_t> pick_counter_{0};
    std::vector<std::unique_ptr<AsyncClient>> clients_;

    // server
    std::unique_ptr<grpc::Server> server_;
    std::thread server_thread_;
    TaskServiceImpl service_;
};

#endif