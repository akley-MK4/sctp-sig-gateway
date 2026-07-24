#include "grpc_wrapper.h"
#include "task.grpc.pb.h"
#include "task.pb.h"

#include <grpcpp/create_channel.h>
#include <grpcpp/client_context.h>

#include <memory>
#include <string>
#include <cstring>

using std::string;
using grpc::ChannelInterface;
using grpc::InsecureChannelCredentials;


class GrpcChannel {
public:
    GrpcChannel(const string& target) {
        channel = grpc::CreateChannel(target, InsecureChannelCredentials());
        std::cout << "[GrpcChannel] Channel created successfully" << std::endl;
    }

    ~GrpcChannel() {
        channel.reset();
        std::cout << "[GrpcChannel] Channel released, destructor complete" << std::endl;
    }
    std::shared_ptr<ChannelInterface> get_channel() {
        return channel;
    }

private:
    std::shared_ptr<ChannelInterface> channel;
};

extern "C" {

void* grpc_create_channel(const char* target) {
    auto* handle = new GrpcChannel(target);
    return handle;
}

int grpc_create_task(void* grpc_channel, const char* task_name, char* error_message) {
    if (grpc_channel == NULL) {
        return 1;
    }

    auto* chan = static_cast<GrpcChannel*>(grpc_channel);

    task::TaskService::Stub stub(chan->get_channel());
    task::MsgCreateTaskRequest request;
    task::MsgCreateTaskResponse response;

    request.set_taskname(task_name);
    // metadata
    task::MsgMetadata* metadata = request.mutable_metadata();
    // timestamp
    auto duration = std::chrono::system_clock::now().time_since_epoch();
    metadata->mutable_timestamp()->set_seconds(std::chrono::duration_cast<std::chrono::seconds>(duration).count());
    // seq
    metadata->set_seq(1);

    grpc::ClientContext context;
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

void grpc_destroy_channel(void* channel) {
    if (!channel) return;
    auto* h = static_cast<GrpcChannel*>(channel);
    delete h;
}

} // extern "C"