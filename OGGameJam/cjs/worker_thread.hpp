#ifndef CJS_WORKER_THREAD_HPP
#define CJS_WORKER_THREAD_HPP
#include "common.hpp"
#include "iqueue.hpp"

namespace cjs {

	class work_queue;

	// contains a worker thread that can work on any iqueue object
	class worker_thread final {
		CJS_NO_COPY(worker_thread);
		CJS_NO_MOVE(worker_thread);
	public:

		worker_thread();
		~worker_thread();

		// attaches it to a queue. set to nullptr to detach
		// will start and stop the worker thread as needed
		void attach_to(iqueue* queue);

	private:

		using work_t = detail::work;

		static void worker(worker_thread* thread);

		iqueue* m_queue;
		atomic_bool m_shouldstop;
		thread m_thread;

	};

}

#include "worker_thread.inl"

#endif // !CJS_WORKER_THREAD_HPP