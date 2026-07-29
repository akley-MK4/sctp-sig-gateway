#include "grpc_async_client.h"
#include <iostream>

AsyncClient::AsyncClient(int id, std::shared_ptr<grpc::ChannelInterface> channel)
    : id_(id), channel_(std::move(channel)) {
    // Create stub from the externally provided channel
    stub_ = task::TaskService::NewStub(channel_);
    // Create a dedicated CompletionQueue for this client
    cq_ = std::make_unique<grpc::CompletionQueue>();
}

AsyncClient::~AsyncClient() {
    Stop();
}

bool AsyncClient::Start() {
    if (running_.exchange(true)) {
        return false;   // Already started
    }
    cq_thread_ = std::thread([this]() { CQThreadFunc(); });
    std::cout << "[INFO] AsyncClient id=" << id_ << " started.\n";
    return true;
}

void AsyncClient::Stop() {
    if (!running_.exchange(false)) {
        return;   // Already stopped
    }

    // 1. Shutdown CQ – unblocks Next() in the background thread
    if (cq_) {
        cq_->Shutdown();
    }

    // 2. Join the CQ thread
    if (cq_thread_.joinable()) {
        cq_thread_.join();
    }

    // 3. Drain any remaining tags to avoid memory leaks
    void* tag = nullptr;
    bool ok = false;
    while (cq_->Next(&tag, &ok)) {
        if (ok && tag) {
            auto* call = static_cast<CallTagBase*>(tag);
            call->OnComplete();
            delete call;
        }
    }

    std::cout << "[INFO] AsyncClient id=" << id_ << " stopped.\n";
}

// ── Core loop: block on CQ, invoke callback directly, delete tag ──
void AsyncClient::CQThreadFunc() {
    void* tag = nullptr;
    bool ok = false;

    while (running_.load()) {
        auto status = cq_->Next(&tag, &ok);

        if (status == grpc::CompletionQueue::SHUTDOWN) {
            break;   // CQ closed, exit loop
        }
        if (!ok || !tag) {
            continue;   // Operation cancelled or null tag, skip
        }

        // Directly cast, call callback, and free – no intermediate queue
        auto* call = static_cast<CallTagBase*>(tag);
        call->OnComplete();
        delete call;
    }
}

// ── Fire-and-forget async CreateTask ──────────────────────────────
void AsyncClient::CreateTaskAsync(const std::string& task_name, asyncCallback cb) {
    auto* call = new CreateTaskCall();
    call->request.set_taskname(task_name);
    call->callback = std::move(cb);
    call->ctx.set_deadline(std::chrono::system_clock::now() +
                           std::chrono::seconds(2));

    auto reader = stub_->AsyncCreateTask(&call->ctx, call->request, cq_.get());
    reader->Finish(&call->response, &call->status, static_cast<void*>(call));
}

// ── Fire-and-forget async DeleteTask ──────────────────────────────
void AsyncClient::DeleteTaskAsync(const std::string& task_name, asyncCallback cb) {
    auto* call = new DeleteTaskCall();
    call->request.set_taskname(task_name);
    call->callback = std::move(cb);
    call->ctx.set_deadline(std::chrono::system_clock::now() +
                           std::chrono::seconds(2));

    auto reader = stub_->AsyncDeleteTask(&call->ctx, call->request, cq_.get());
    reader->Finish(&call->response, &call->status, static_cast<void*>(call));
}