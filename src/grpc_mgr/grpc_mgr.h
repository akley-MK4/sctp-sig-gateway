#ifndef GRPC_MGR_H_
#define GRPC_MGR_H_

#include <thread>
#include <grpcpp/create_channel.h>
#include "grpc_svc_impl.h"

using std::string;
using grpc::ChannelInterface;

class GrpcMgr {
public:
  static GrpcMgr& GetInstance();
  int initialize();
  int Start();
  void startServer();
  int CreateTask(const char* task_name, char* error_message);
  bool IsStarted();
  std::shared_ptr<ChannelInterface> getChannel();

private:
  GrpcMgr();
  ~GrpcMgr();
  void ServerThreadFunc();

  bool initialized_;
  bool started_;
  string target_addr_;
  string srv_addr_;
  std::shared_ptr<ChannelInterface> channel_;
  std::unique_ptr<grpc::Server> server_;
  std::thread server_thread_;

  TaskServiceImpl service_;
};

#endif