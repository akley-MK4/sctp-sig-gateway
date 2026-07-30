#include "grpc_export.h"
#include "grpc_mgr.h"

int initialize_grpc_mgr() {
    GrpcMgr& inst = GrpcMgr::GetInstance();
    return inst.initialize();
}

int stop_grpc_mgr() {
    GrpcMgr& inst = GrpcMgr::GetInstance();
    return inst.stop();
}

int start_grpc_mgr() {
    GrpcMgr& inst = GrpcMgr::GetInstance();
    return inst.start();
}

int grpc_create_task(const char* task_name, char* error_message) {
    GrpcMgr& inst = GrpcMgr::GetInstance();
    if (!inst.IsStarted()) {
        return 1;
    }

    task::TaskService::Stub stub(inst.getChannel());
    auto request = task::MsgCreateTaskRequest();
    task::MsgCreateTaskResponse response;

    request.set_taskname(task_name);
    // metadata
    task::MsgMetadata* metadata = request.mutable_metadata();
    // timestamp
    auto duration = std::chrono::system_clock::now().time_since_epoch();
    metadata->mutable_timestamp()->set_seconds(std::chrono::duration_cast<std::chrono::seconds>(duration).count());
    // seq
    metadata->set_seq(1);

    // ctx for timeout
    grpc::ClientContext context;
    context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(2));

    grpc::Status status = stub.CreateTask(&context, request, &response);

    if (!status.ok()) {
        error_message = strdup(status.error_message().c_str());
        return 1;
    }

    if (response.errcode() != 0) {
        error_message = strdup(("Task creation failed with error code: " + std::to_string(response.errcode())).c_str());
        return 1;
    }

    return 0;
}

int grpc_create_task_async(const char* task_name) {
    GrpcMgr& inst = GrpcMgr::GetInstance();
    if (!inst.IsStarted()) {
        return 1;
    }

    auto asyncClient = inst.PickClient();
    asyncClient->CreateTaskAsync(string(task_name), [](int errcode, void* reply) {
        if (errcode == 0) {
            auto* resp = static_cast<task::MsgCreateTaskResponse*>(reply);
            std::cout << "Task created successfully." + resp->metadata().DebugString() << std::endl;
        } else {
            std::cerr << "Task creation failed, error code: " << errcode << std::endl;
        }
    });


    return 0;
}
