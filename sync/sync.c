#include "sync.h"
#include "log.h"

struct synchronization_context *sync_context;

void set_sync(struct synchronization_context *synchronization_context) {
	sync_context = synchronization_context;
}

struct mutex *mutex_create(int blocking)
{
	struct mutex *m = (sync_context->alloc_mutex)();
	if (m == NULL)
		return NULL;
	m->blocking = blocking;
	m->locked = 0;
	if (blocking) {
		// blocking mutex need wait queue but spinning mutex not
		init_queue(&m->wait_queue, WAIT_QUEUE_MAX_LENGTH,
			   m->_wait_queue_data);
	}
	return m;
}

void mutex_lock(struct mutex *m)
{
	if (!m->locked) {
		m->locked = 1;
		debugf("lock a free mutex");
		return;
	}
	if (!m->blocking) {
		// spin mutex will just poll
		debugf("try to lock spin mutex");
		while (m->locked) {
			(sync_context->yield)();
		}
		m->locked = 1;
		debugf("lock spin mutex after some trials");
		return;
	}
	// blocking mutex will wait in the queue
	push_queue(&m->wait_queue, (sync_context->curr_task_id)());
	debugf("block to wait for mutex");
	(sync_context->sleeping)();
	debugf("blocking mutex passed to me");
	// here lock is released (with locked = 1) and passed to me, so just do nothing
}

void mutex_unlock(struct mutex *m)
{
	if (m->blocking) {
		if (is_empty(&m->wait_queue)) {
			// Without waiting thread, just release the lock
			m->locked = 0;
			debugf("blocking mutex released");
		} else {
			// Or we should give lock to next thread
			int t = pop_queue(&m->wait_queue);
			(sync_context->running)(t);
			debugf("blocking mutex passed to thread %d", t);
		}
	} else {
		m->locked = 0;
		debugf("spin mutex unlocked");
	}
}

struct semaphore *semaphore_create(int count)
{
	struct semaphore *s = (sync_context->alloc_semaphore)();
	if (s == NULL)
		return NULL;
	s->count = count;
	init_queue(&s->wait_queue, WAIT_QUEUE_MAX_LENGTH, s->_wait_queue_data);
	return s;
}

void semaphore_up(struct semaphore *s)
{
	s->count++;
	if (s->count <= 0) {
		// count <= 0 after up means wait queue not empty
		if (is_empty(&s->wait_queue)) {
			panic("count <= 0 after up but wait queue is empty?");
		}
		int t = pop_queue(&s->wait_queue);
		(sync_context->running)(t);
		debugf("semaphore up and notify another task");
	}
	debugf("semaphore up from %d to %d", s->count - 1, s->count);
}

void semaphore_down(struct semaphore *s)
{
	s->count--;
	if (s->count < 0) {
		// s->count < 0 means need to wait (state=SLEEPING)
		push_queue(&s->wait_queue, (sync_context->curr_task_id)());
		debugf("semaphore down to %d and wait...", s->count);
		(sync_context->sleeping)();
		debugf("semaphore up to %d and wake up", s->count);
	}
	debugf("finish semaphore_down with count = %d", s->count);
}

struct condvar *condvar_create()
{
	struct condvar *c = (sync_context->alloc_codvar)();
	if (c == NULL)
		return NULL;
	init_queue(&c->wait_queue, WAIT_QUEUE_MAX_LENGTH, c->_wait_queue_data);
	return c;
}

void cond_signal(struct condvar *cond)
{
	if (!is_empty(&cond->wait_queue)) {
		int t = pop_queue(&cond->wait_queue);
		(sync_context->running)(t);
		debugf("signal wake up thread %d", t);
	} else {
		debugf("dummpy signal");
	}
}

void cond_wait(struct condvar *cond, struct mutex *m)
{
	// conditional variable will unlock the mutex first and lock it again on return
	mutex_unlock(m);
	// now just wait for cond
	push_queue(&cond->wait_queue, (sync_context->curr_task_id)());
	debugf("wait for cond");
	(sync_context->sleeping)();
	debugf("wake up from cond");
	mutex_lock(m);
}
