#ifndef GRPC_SVC_IMPL_H_
#define GRPC_SVC_IMPL_H_


#include <grpcpp/grpcpp.h>
#include "task.grpc.pb.h"

class TaskServiceImpl final : public task::TaskService::CallbackService {
public:
    grpc::ServerUnaryReactor* CreateTask(
        grpc::CallbackServerContext* context,
        const task::MsgCreateTaskRequest* request,
        task::MsgCreateTaskResponse* response
    ) override;

    grpc::ServerUnaryReactor* DeleteTask(
        grpc::CallbackServerContext* context,
        const task::MsgDeleteTaskRequest* request,
        task::MsgDeleteTaskResponse* response
    ) override;
};

#endif