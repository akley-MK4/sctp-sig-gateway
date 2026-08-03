#ifndef GRPC_ASYNC_CLIENT_H_H
#define GRPC_ASYNC_CLIENT_H_H

#include <functional>
#include <grpcpp/grpcpp.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/support/status.h>
#include <thread>
#include <atomic>
#include <memory>
#include <functional>
#include <string>
#include "task.grpc.pb.h"   // generated proto header
#include "grpc_export.h"  // for callback type

using std::string;
//using createTaskAsyncCallbackk = std::function<void(int errCode, int createTaskId)>;
using deleteTaskAsyncCallback = std::function<void(int errCode, void* response)>;

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
    void CreateTaskAsync(const string& task_name, grpc_create_task_callback cb);
    void CreateTaskAsyncWithCallbackApi(const string& task_name, grpc_create_task_callback cb);

    void DeleteTaskAsync(const string& task_name, deleteTaskAsyncCallback cb);

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

// ── Base class for all AsyncCall tags ──────────────────────────────
struct CallTagBase {
    virtual ~CallTagBase() = default;
    virtual void OnComplete() = 0;
};

// ── CreateTask RPC ─────────────────────────────────────────────────
struct CreateTaskCall : public CallTagBase {
    grpc::ClientContext ctx;
    grpc::Status status;
    task::MsgCreateTaskRequest  request;
    task::MsgCreateTaskResponse response;
    grpc_create_task_callback callback;

    void OnComplete() override {
        callback(status.error_code(), response.createtaskid());
    }
};

// ── DeleteTask RPC ─────────────────────────────────────────────────
struct DeleteTaskCall : public CallTagBase {
    grpc::ClientContext ctx;
    grpc::Status status;
    task::MsgDeleteTaskRequest  request;
    task::MsgDeleteTaskResponse response;
    deleteTaskAsyncCallback callback;

    void OnComplete() override {
        int err = status.ok() ? 0 : status.error_code();
        if (callback) callback(err, &response);
    }
};

#endif