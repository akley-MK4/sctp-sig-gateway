#include "logger.h"
#include "grpc_svc_impl.h"

grpc::ServerUnaryReactor* TaskServiceImpl::CreateTask(
    grpc::CallbackServerContext* context,
    const task::MsgCreateTaskRequest* request,
    task::MsgCreateTaskResponse* response
) {
    task::MsgMetadata reqMetadata = request->metadata();
    log_debug("[TaskService] Create task: name=%s, seq=%lu, time=%ld",
              request->taskname().c_str(),
              reqMetadata.seq(),
              reqMetadata.timestamp().seconds());
    
    task::MsgMetadata *respMetadata = response->mutable_metadata();
    respMetadata->set_seq(reqMetadata.seq());
    // timestamp
    auto duration = std::chrono::system_clock::now().time_since_epoch();
    respMetadata->mutable_timestamp()->set_seconds(std::chrono::duration_cast<std::chrono::seconds>(duration).count());
    // task id
    response->set_createtaskid(12345);  // Example task ID

    response->set_errcode(0);

    log_info("[TaskService] Sending the CreateTask response message");

    auto* reactor = context->DefaultReactor();

    // testing
    reactor->Finish(grpc::Status::OK);
    return reactor;
}

grpc::ServerUnaryReactor* TaskServiceImpl::DeleteTask(
    grpc::CallbackServerContext* context,
    const task::MsgDeleteTaskRequest* request,
    task::MsgDeleteTaskResponse* response
) {
    log_debug("[TaskService] Delete task: name=%s, seq=%lu, time=%ld",
              request->taskname().c_str(),
              request->metadata().seq(),
              request->metadata().timestamp().seconds());

    response->mutable_metadata()->set_seq(request->metadata().seq());
    response->set_errcode(0);

    log_info("[TaskService] Sending the DeleteTask response message");
    
    auto* reactor = context->DefaultReactor();
    return reactor;
}