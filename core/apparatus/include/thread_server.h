#pragma once

typedef u32 thread_id_t;
constexpr thread_id_t NULL_THREAD = UINT32_MAX;

typedef std::function<void()> Thread_Task;

struct Thread_Context {
	AP_API Thread_Context() = default;
	AP_API Thread_Context(const Thread_Context& src) {
		task_queue = src.task_queue;
		pause.store(src.pause.load());
		kill.store(src.kill.load());
	}
	AP_API Thread_Context(Thread_Context&& src) noexcept {
		task_queue = std::move(src.task_queue);
		pause.store(src.pause.load());
		kill.store(src.kill.load());
	}
	Thread_Safe_Queue<Thread_Task> task_queue;
	std::atomic<bool> pause;
	std::atomic<bool> kill = false;

	std::thread::id id;
};

struct Thread_Server {
	AP_API Thread_Server();
	AP_API ~Thread_Server();
	AP_API thread_id_t make_thread(bool start = true);

	AP_API void pause_thread(thread_id_t tid);
	AP_API void start_thread(thread_id_t tid);

	AP_API void kill_thread(thread_id_t tid);

	AP_API void queue_task(thread_id_t tid, const Thread_Task& task);

	AP_API void wait_for_thread(thread_id_t tid);

	AP_API bool is_thread_busy(thread_id_t tid);
	
	Dynamic_Array<Thread_Context*> _thread_contexts; 
};