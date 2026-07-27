#include "grpc_svc_impl.h"

grpc::Status TaskServiceImpl::CreateTask(
    grpc::ServerContext* context,
    const task::MsgCreateTaskRequest* request,
    task::MsgCreateTaskResponse* response
) {
    task::MsgMetadata reqMetadata = request->metadata();

    std::cout << "[TaskService] Create task: name="
              << request->taskname()
              << ", seq=" << request->metadata().seq()
              << ", time=" << request->metadata().timestamp().seconds()
              << std::endl;

    
    task::MsgMetadata *respMetadata = response->mutable_metadata();
    respMetadata->set_seq(reqMetadata.seq());
    // timestamp
    auto duration = std::chrono::system_clock::now().time_since_epoch();
    respMetadata->mutable_timestamp()->set_seconds(std::chrono::duration_cast<std::chrono::seconds>(duration).count());

    response->set_errcode(0);

    std::cout << "[Server] Sending response message" << std::endl;
    return grpc::Status::OK;
}

grpc::Status TaskServiceImpl::DeleteTask(
    grpc::ServerContext* context,
    const task::MsgDeleteTaskRequest* request,
    task::MsgDeleteTaskResponse* response
) {
    std::cout << "[TaskService] Delete task: name=" << request->taskname() << std::endl;

    response->mutable_metadata()->set_seq(request->metadata().seq());
    response->set_errcode(0);

    std::cout << "[Server] Sending response message" << std::endl;
    return grpc::Status::OK;
}