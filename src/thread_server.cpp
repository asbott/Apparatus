#include "pch.h"

#include "thread_server.h"

void work(thread_id_t tid, Dynamic_Array<Thread_Context>* pcontexts) {
	auto& contexts = *pcontexts;
	while (!contexts[tid].kill) {
		while (contexts[tid].pause);
		auto cmd = contexts[tid].task_queue.front();
		if (cmd.has_value()) {
			cmd.value()();
			contexts[tid].task_queue.pop();
		}
	}
	contexts[tid].dead = true;
}

Thread_Server::Thread_Server() {
	_thread_contexts.reserve(MAX_NUMBER_OF_THREADS);
}
Thread_Server::~Thread_Server() {
	for (int i = 0; i < _thread_contexts.size(); i++) {
		wait_for_thread(i);
		_thread_contexts[i].kill = true;
	}
}

thread_id_t Thread_Server::make_thread(bool start) {
	thread_id_t tid = NULL_THREAD;
	for (thread_id_t i = 0; i < _thread_contexts.size(); i++) {
		if (_thread_contexts[i].dead) {
			_thread_contexts[i].dead = false;
			_thread_contexts[i].kill = false;
			_thread_contexts[i].pause = !start;
			_thread_contexts[i].task_queue.clear();
			tid = i;
			break;
		}
	}
	if (tid == NULL_THREAD) {
		tid = (thread_id_t)_thread_contexts.size();
		Thread_Context ctx;
		ctx.pause = !start;
		_thread_contexts.push_back(ctx);
	}
	
	std::thread(work, tid, &_thread_contexts).detach();
	return tid;
}

void Thread_Server::pause_thread(thread_id_t tid) {
	_thread_contexts[tid].pause = true;
}

void Thread_Server::start_thread(thread_id_t tid) {
	_thread_contexts[tid].pause = false;
}

void Thread_Server::kill_thread(thread_id_t tid) {
	_thread_contexts[tid].kill = true;
}

void Thread_Server::queue_task(thread_id_t tid, const Thread_Task& task) {
	auto& ctx = _thread_contexts[tid];
	ctx.task_queue.push(task);
}

void Thread_Server::wait_for_thread(thread_id_t tid) {
	auto& ctx = _thread_contexts[tid];
	while (ctx.task_queue.size() > 0);
}

bool Thread_Server::is_thread_busy(thread_id_t tid) {
	return _thread_contexts[tid].task_queue.size() > 0;
}
