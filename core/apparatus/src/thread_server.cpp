#include "pch.h"

#include "thread_server.h"

void work(Thread_Context* pctx) {
	while (!pctx->kill) {
		while (pctx->pause && !pctx->kill);
		auto cmd = pctx->task_queue.front();
		while (cmd.has_value() && !pctx->kill) {
			cmd.value()();
			pctx->task_queue.pop();
			cmd = pctx->task_queue.front();
		}
	}

	delete pctx;
}

Thread_Server::Thread_Server() {
	
}
Thread_Server::~Thread_Server() {
	for (int i = 0; i < _thread_contexts.size(); i++) {
		if (_thread_contexts[i]) kill_thread(i);
	}
}

thread_id_t Thread_Server::make_thread(bool start) {
	thread_id_t tid = NULL_THREAD;
	for (thread_id_t i = 0; i < _thread_contexts.size(); i++) {
		if (!_thread_contexts[i]) {
			tid = i;
			Thread_Context* ctx = new Thread_Context();
			ctx->pause = !start;
			_thread_contexts[i] = ctx;
			break;
		}
	}
	if (tid == NULL_THREAD) {
		tid = (thread_id_t)_thread_contexts.size();
		Thread_Context* ctx = new Thread_Context();
		ctx->pause = !start;
		_thread_contexts.push_back(ctx);
	}
	
	auto t = std::thread(work, _thread_contexts[tid]);
	log_info("Created a thread '{}' with os id '{}'", tid, t.get_id());
	_thread_contexts[tid]->id = t.get_id();
	t.detach();
	if (tid >= std::thread::hardware_concurrency()) {
		log_warn("{}th thread created, but system only has a hardware concurrency of {}",
					tid + 1, std::thread::hardware_concurrency());
	} else {
		log_info("{} physical threads remaining ({}/{} used)", std::thread::hardware_concurrency() - (tid + 1), tid + 1, std::thread::hardware_concurrency());
	}
	return tid;
}

void Thread_Server::pause_thread(thread_id_t tid) {
	_thread_contexts[tid]->pause = true;
}

void Thread_Server::start_thread(thread_id_t tid) {
	_thread_contexts[tid]->pause = false;
}

void Thread_Server::kill_thread(thread_id_t tid) {
	_thread_contexts[tid]->kill = true;
	_thread_contexts[tid]->pause = false;
	_thread_contexts[tid] = NULL;
}

void Thread_Server::queue_task(thread_id_t tid, const Thread_Task& task) {
	auto& ctx = _thread_contexts[tid];
	ctx->task_queue.push(task);
}

void Thread_Server::wait_for_thread(thread_id_t tid) {
	if (std::this_thread::get_id() == _thread_contexts[tid]->id) return;
	auto& ctx = _thread_contexts[tid];
	if (ctx->pause) return;
	while (ctx->task_queue.size() > 0);
}

bool Thread_Server::is_thread_busy(thread_id_t tid) {
	return _thread_contexts[tid]->task_queue.size() > 0;
}
