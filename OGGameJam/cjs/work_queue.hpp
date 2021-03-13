#ifndef CJS_WORK_QUEUE_HPP
#define CJS_WORK_QUEUE_HPP
#include "common.hpp"
#include "ijob.hpp"
#include "fence.hpp"
#include "detail\work.hpp"
#include "iqueue.hpp"

namespace cjs {

	class work_queue final : public iqueue {
		CJS_NO_COPY(work_queue);
		CJS_NO_MOVE(work_queue);
	public:

		// constructs the work queue with the specified number of nodes in a pool
		work_queue(size_t minpoolsize = 0);

		~work_queue();

		// returns the amount of work
		size_t size();

		// returns the number of nodes in the pool
		size_t pool_size();

		// returns the total number of nodes attached to this queue
		size_t total_size();

		// submitting jobs

		// submit a job to be worked on
		void submit(ijob* job_object);

		// submit a function to call
		void submit(void(*job_func)(void*), void* value = nullptr);

		// submits a fence to stop the threads
		// will only block as many threads as there are at the time of submission
		void submit(ifence* fence_object);

	private:

		using work_t = cjs::detail::work;
		using mutex = std::mutex;
		using mutex_guard = std::lock_guard<std::mutex>;
		using atomic_bool = std::atomic_bool;
		using atomic_int32_t = std::atomic_int32_t;

		struct work_node {
			work_node* next;
			work_t work;
		};

		work_t pop_work();
		void push_work(work_t work);
		work_node* m_worklist_front;
		work_node* m_worklist_back;
		size_t m_worklist_sz;
		mutex m_worklist_lock;

		work_node* get_or_make_node();
		work_node* m_nodepool;
		size_t m_nodepool_sz;
		mutex m_nodepool_lock;

		mutex m_worker_lock;
		std::vector<worker_thread*> m_workers;

		work_t _get_work() override;
		void _add_worker(worker_thread* worker) override;
		void _remove_worker(worker_thread* worker) override;
	};

}

#include "work_queue.inl"

#endif // !CJS_WORK_QUEUE_HPP