#include "processor.h"

void* current_task;
struct manager* task_manager;

/// manager各个接口的实现在各个lab中，在这里初始化proc_manager
void set_manager(struct manager* lab_manager) {
	task_manager = lab_manager;
}

/// 获得当前正在运行的进程
void* curr_task() {
	return current_task;
}

/// 修改当前正在运行的进程
void set_curr(void* p) {
	current_task = p;
}

/// 根据id获得进程
void* get_task(int id) {
	return task_manager->get(id);
}

/// 根据进程获得id
int get_id(void* p) {
	return task_manager->change(p);
}

/// 创建进程
void* alloc_task() {
	return task_manager->create();
}

/// 释放进程
void free_task(void* p) {
	task_manager->remove(p);
}

/// 阻塞进程
void add_task(void* p) {
	task_manager->add(p);
}

/// 调度进程
void* fetch_task() {
	return task_manager->fetch();
}