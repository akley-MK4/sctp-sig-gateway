#include "logger.h"
#include "grpc_async_client.h"
#include <iostream>

AsyncClient::AsyncClient(int id, std::shared_ptr<grpc::ChannelInterface> channel)
    : id_(id), channel_(std::move(channel)) {
    // Create stub from the externally provided channel
    stub_ = task::TaskService::NewStub(channel_);
    // Create a dedicated CompletionQueue for this client
    cq_ = std::make_unique<grpc::CompletionQueue>();
    running_.store(false);
}

AsyncClient::~AsyncClient() {
    Stop();
}

bool AsyncClient::Start() {
    if (running_.exchange(true)) {
        log_warn("[AsyncClient] Start() called but already running, id=%d", id_);
        return false;   // Already started
    }

    try {
        // Launch the CQ thread and report its id for diagnostics
        cq_thread_ = std::thread(&AsyncClient::CQThreadFunc, this);
    } catch (const std::system_error& e) {
        // Failed to create thread; restore running_ flag and report error
        //running_.store(false);
        log_error("[AsyncClient] id=%d failed to start thread: %s", id_, e.what());
        return false;
    }

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

    log_info("[AsyncClient] id=%d stopped.", id_);
}

// ── Core loop: block on CQ, invoke callback directly, delete tag ──
void AsyncClient::CQThreadFunc() {
    log_info("[AsyncClient] CQ thread started. id=%d", id_);
    void* tag = nullptr;
    bool ok = false;

    while (running_.load()) {
        auto status = cq_->Next(&tag, &ok);

        if (status == grpc::CompletionQueue::SHUTDOWN) {
            log_info("[AsyncClient] id=%d CQ closed, exit loop.", id_);
            break;   // CQ closed, exit loop
        }
        if (!ok || !tag) {
            continue;   // Operation cancelled or null tag, skip
        }

        // Directly cast, call callback, and free – no intermediate queue
        log_debug("[AsyncClient] received response from target grpc server. id=%d", id_);
        auto* call = static_cast<CallTagBase*>(tag);
        call->OnComplete();
        delete call;
    }
}

// ── Fire-and-forget async CreateTask ──────────────────────────────
void AsyncClient::CreateTaskAsync(const std::string& task_name, grpc_create_task_callback cb) {
    auto* call = new CreateTaskCall();
    call->request.set_taskname(task_name);
    call->callback = cb;
    call->ctx.set_deadline(std::chrono::system_clock::now() +
                           std::chrono::seconds(2));

    auto reader = stub_->AsyncCreateTask(&call->ctx, call->request, cq_.get());
    reader->Finish(&call->response, &call->status, static_cast<void*>(call));
    log_debug("[AsyncClient] sent request to target grpc server. id=%d", id_);
}

void AsyncClient::CreateTaskAsyncWithCallbackApi(const std::string& task_name, grpc_create_task_callback cb) {
    auto* call = new CreateTaskCall();
    call->request.set_taskname(task_name);
    call->callback = cb;
    call->ctx.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(2));
    stub_->async()->CreateTask(&call->ctx, &call->request, &call->response,
        [call](grpc::Status status) {
            call->status = status;
            call->OnComplete();
            delete call;
        });
}

// ── Fire-and-forget async DeleteTask ──────────────────────────────
void AsyncClient::DeleteTaskAsync(const std::string& task_name, deleteTaskAsyncCallback cb) {
    auto* call = new DeleteTaskCall();
    call->request.set_taskname(task_name);
    call->callback = std::move(cb);
    call->ctx.set_deadline(std::chrono::system_clock::now() +
                           std::chrono::seconds(2));

    auto reader = stub_->AsyncDeleteTask(&call->ctx, call->request, cq_.get());
    reader->Finish(&call->response, &call->status, static_cast<void*>(call));
}