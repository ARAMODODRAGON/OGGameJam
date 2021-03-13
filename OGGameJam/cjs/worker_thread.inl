

inline cjs::worker_thread::worker_thread()
	: m_queue(nullptr), m_shouldstop(true) { }

inline cjs::worker_thread::~worker_thread() {
	if (m_queue) {
		m_shouldstop = true;
		m_thread.join();
		m_queue->_remove_worker(this);
	}
}

inline void cjs::worker_thread::attach_to(iqueue* queue) {
	if (queue == m_queue) return;

	if (m_queue) {
		m_shouldstop = true;
		m_thread.join();
		m_queue->_remove_worker(this);
		m_queue = nullptr;
	}

	if (queue) {
		queue->_add_worker(this);
		m_queue = queue;
		m_shouldstop = false;
		m_thread = thread(&worker_thread::worker, this);
	}
}

inline void cjs::worker_thread::worker(worker_thread* thread) {
	while (!thread->m_shouldstop) {
		if (auto* q = thread->m_queue) {
			work_t work = q->_get_work();

			switch (work.type) {
				case work_t::type_func:
					work.func(work.func_val);
					break;
				case work_t::type_object:
					work.object->execute();
					break;
				case work_t::type_fence:
					work.fence->_join();
					break;
				default: break;
			}
		}
	}
}
