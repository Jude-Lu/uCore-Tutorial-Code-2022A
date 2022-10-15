#include "processor.h"

void* current_task;
struct manager* task_manager;

void set_manager(struct manager* lab_manager) {
	task_manager = lab_manager;
}

void* curr_task() {
	return current_task;
}

void set_curr(void* p) {
	current_task = p;
}

void* get_task(int id) {
	return task_manager->get(id);
}

void* alloc_task() {
	return task_manager->create();
}

void free_task(void* p) {
	task_manager->remove(p);
}

void add_task(void* p) {
	task_manager->add(p);
}

void* fetch_task() {
	return task_manager->fetch();
}