#ifndef CJS_IQUEUE_HPP
#define CJS_IQUEUE_HPP
#include "detail\work.hpp"

namespace cjs {

	class worker_thread;

	// all contaned functions are invoked by the worker thread object
	// used to attach workers to a queue
	class iqueue {
		using work_t = cjs::detail::work;
	public:
		virtual work_t _get_work() = 0;
		virtual void _add_worker(worker_thread* worker) = 0;
		virtual void _remove_worker(worker_thread* worker) = 0;
	};

}

#endif // !CJS_IQUEUE_HPP