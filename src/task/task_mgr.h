#ifndef TASK_MGR_H_
#define TASK_MGR_H_

#include "event_timer.h"

typedef struct task_mgr_s {
    int is_server;
    int running;
    struct event* timer;

}task_mgr_t;

int initialize_task_mgr(int is_server);
int start_task_mgr();
int create_task(const char* task_name);

#endif