#pragma once

#include <grpcpp/grpcpp.h>
#include <grpcpp/channel.h>
#include <thread>
#include <atomic>
#include <memory>
#include <functional>
#include <string>
#include "call_tag.h"
#include "task.grpc.pb.h"   // generated proto header

// Single async client: owns one CompletionQueue and one background thread.
// The Channel is passed in from outside (e.g. GrpcMgr), not created internally.
class AsyncClient {
public:
    // Construct with an integer id and a shared Channel.
    AsyncClient(int id, std::shared_ptr<grpc::ChannelInterface> channel);
    ~AsyncClient();

    // Disable copy, allow move semantics if needed (deleted for simplicity)
    AsyncClient(const AsyncClient&) = delete;
    AsyncClient& operator=(const AsyncClient&) = delete;

    bool Start();   // Launch the CQ drain thread
    void Stop();    // Shutdown CQ and join the thread

    // Non-blocking async RPC interfaces
    void CreateTaskAsync(const std::string& task_name, asyncCallback cb);

    void DeleteTaskAsync(const std::string& task_name, asyncCallback cb);

    int Id() const { return id_; }

private:
    void CQThreadFunc();   // Background thread body

    int id_;
    std::shared_ptr<grpc::ChannelInterface> channel_;
    std::shared_ptr<task::TaskService::Stub> stub_;
    std::unique_ptr<grpc::CompletionQueue> cq_;
    std::atomic<bool> running_{false};
    std::thread cq_thread_;
};