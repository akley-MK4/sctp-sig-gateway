#ifndef TASK_MGR_H_
#define TASK_MGR_H_

typedef struct task_mgr_s {

}task_mgr_t;

int initialize_task_mgr();
int start_task_mgr();
int create_task(const char* task_name);

#endif