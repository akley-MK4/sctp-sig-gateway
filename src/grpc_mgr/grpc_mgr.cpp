#include "grpc_mgr.h"
#include "grpc_svc_impl.h"
#include "grpc_wrapper.h"

GrpcMgr::GrpcMgr() 
  : initialized_(false), 
    started_(false) 
{

}

GrpcMgr::~GrpcMgr() {
    if (!initialized_) {
        return;
    }

    channel_.reset();
}

GrpcMgr& GrpcMgr::GetInstance() {
    static GrpcMgr instance;
    return instance;
}

int wrap_initialize_grpc_mgr(const char* target_addr, const char* srv_addr) {
    GrpcMgr& inst = GrpcMgr::GetInstance();
    return inst.initialize(target_addr, srv_addr);
}

int wrap_start_grpc_mgr() {
    GrpcMgr& inst = GrpcMgr::GetInstance();
    return inst.Start();
}

int GrpcMgr::initialize(const string& target_addr, const string& srv_addr) {
    target_addr_ = target_addr;
    srv_addr_ = srv_addr;
    channel_ = grpc::CreateChannel(target_addr, grpc::InsecureChannelCredentials());
    initialized_ = true;
    return 0;
}

int GrpcMgr::Start() {
    if (!initialized_) {
        return 1;
    }
    if (started_) {
        return 1;
    }
    started_ = true;

    startServer();
    return 0;
}

std::shared_ptr<ChannelInterface> GrpcMgr::getChannel() {
    return channel_;
}

bool GrpcMgr::IsStarted() {
    return started_;
}

void GrpcMgr::startServer() {
    //string server_address("0.0.0.0:50051");

    grpc::ServerBuilder builder;
    builder.AddListeningPort(srv_addr_, grpc::InsecureServerCredentials());
    builder.RegisterService(&service_);

    server_ = builder.BuildAndStart();
    std::cout << "[Server] Listening on " << srv_addr_ << std::endl;

    //server->Wait();

    server_thread_ = std::thread(&GrpcMgr::ServerThreadFunc, this);
}

void GrpcMgr::ServerThreadFunc() {
    std::cout << "[GrpcMgr] Server thread started, waiting for requests..." << std::endl;
    server_->Wait();
    std::cout << "[GrpcMgr] Server thread exited" << std::endl;
}

int create_task(const char* task_name, char* error_message) {
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
