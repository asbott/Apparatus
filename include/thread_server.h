#pragma once

typedef u32 thread_id_t;
constexpr thread_id_t NULL_THREAD = (thread_id_t)0 - (thread_id_t)1;

typedef std::function<void()> Thread_Task;

struct Thread_Context {
	Thread_Safe_Queue<Thread_Task> task_queue;
	bool pause;
	bool kill = false;
	bool dead = false;
};

struct Thread_Server {
	Thread_Server();
	~Thread_Server();
	AP_API thread_id_t make_thread(bool start = true);

	AP_API void pause_thread(thread_id_t tid);
	AP_API void start_thread(thread_id_t tid);

	AP_API void kill_thread(thread_id_t tid);

	AP_API void queue_task(thread_id_t tid, const Thread_Task& task);

	AP_API void wait_for_thread(thread_id_t tid);

	AP_API bool is_thread_busy(thread_id_t tid);
	
	Dynamic_Array<Thread_Context> _thread_contexts; 
};