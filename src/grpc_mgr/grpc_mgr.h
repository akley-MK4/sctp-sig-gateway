#ifndef GRPC_MGR_H_
#define GRPC_MGR_H_

#include <grpcpp/create_channel.h>

using std::string;
using grpc::ChannelInterface;

class GrpcMgr {
public:
  static GrpcMgr& GetInstance();
  int initialize(const string& target_addr, const string& srv_addr);
  int Start();
  void startServer();
  int CreateTask(const char* task_name, char* error_message);

private:
  bool initialized_;
  bool started_;
  GrpcMgr();
  ~GrpcMgr();
  string target_addr;
  string srv_addr;
  std::shared_ptr<ChannelInterface> channel;
};

#endif