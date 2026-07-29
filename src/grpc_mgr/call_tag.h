#pragma once
#include <functional>
#include <string>
#include <grpcpp/client_context.h>
#include <grpcpp/support/status.h>
#include "task.grpc.pb.h"   // your generated proto header

using asyncCallback = std::function<void(int, void*)>;

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
    asyncCallback callback;

    void OnComplete() override {
        callback(status.error_code(), &response);
    }
};

// ── DeleteTask RPC ─────────────────────────────────────────────────
struct DeleteTaskCall : public CallTagBase {
    grpc::ClientContext ctx;
    grpc::Status status;
    task::MsgDeleteTaskRequest  request;
    task::MsgDeleteTaskResponse response;
    asyncCallback callback;

    void OnComplete() override {
        int err = status.ok() ? 0 : status.error_code();
        if (callback) callback(err, &response);
    }
};

// ── To add new RPCs later, simply derive a new class.               ──
// ── Zero changes required in the CQ thread logic.                   ──