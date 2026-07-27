#ifndef GRPC_MGR_H_
#define GRPC_MGR_H_

#include <grpcpp/create_channel.h>

using std::string;
using grpc::ChannelInterface;

class GrpcMgr {
public:
  static int initialize(const string& target_addr, const string& srv_addr);
  static GrpcMgr* GetInstance();
  int Start();
  void startServer();
  int CreateTask(const char* task_name, char* error_message);

private:
  GrpcMgr(const string& target_addr, const string& srv_addr);
  ~GrpcMgr();
  static GrpcMgr* instance;
  string target_addr;
  string srv_addr;
  std::shared_ptr<ChannelInterface> channel;
};

#endif