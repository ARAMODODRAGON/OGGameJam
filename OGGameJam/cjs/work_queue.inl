//#include "work_queue.hpp"

namespace cjs {

	inline work_queue::work_queue(size_t minpoolsize)
		: m_worklist_front(nullptr), m_worklist_back(nullptr), m_worklist_sz(0)
		, m_nodepool(nullptr), m_nodepool_sz(minpoolsize) {
		for (size_t i = 0; i < minpoolsize; i++) {
			work_node* node = new work_node();
			node->next = m_nodepool;
			m_nodepool = node;
		}
	}

	inline work_queue::~work_queue() {
		// empty worklist (moves the nodes into the pool)
		while (m_worklist_sz > 0) {
			pop_work();
		}

		// clean up pool
		mutex_guard mg(m_nodepool_lock);
		while (m_nodepool) {
			work_node* node = m_nodepool;
			m_nodepool = m_nodepool->next;
			delete node;
		}
		m_nodepool_sz = 0;
	}

	inline size_t work_queue::size() {
		mutex_guard mg0(m_worklist_lock);
		return m_worklist_sz;
	}

	inline size_t work_queue::pool_size() {
		mutex_guard mg(m_nodepool_lock);
		return m_nodepool_sz;
	}

	inline size_t work_queue::total_size() {
		mutex_guard mg0(m_worklist_lock);
		mutex_guard mg1(m_nodepool_lock);
		return m_worklist_sz + m_nodepool_sz;
	}

	inline void work_queue::submit(ijob* job_object) {
		work_t work;
		work.object = job_object;
		work.type = work_t::type_object;
		push_work(work);
	}

	inline void work_queue::submit(void(*job_func)(void*), void* value) {
		work_t work;
		work.func = job_func;
		work.func_val = value;
		work.type = work_t::type_func;
		push_work(work);
	}

	inline void work_queue::submit(ifence* fence_object) {
		work_t work;
		work.fence = fence_object;
		work.thread_count = m_workers.size() - 1;
		work.type = work_t::type_fence;
		work.fence->_submit();
		push_work(work);
	}

	inline work_queue::work_t work_queue::pop_work() {
		mutex_guard mg0(m_worklist_lock);
		// empty list, return no work
		if (m_worklist_sz == 0) return work_t();

		// get front node
		work_t work = m_worklist_front->work;
		if (work.thread_count > 0)
			m_worklist_front->work.thread_count--;
		else {
			work_node* node = m_worklist_front;
			m_worklist_front = node->next;
			--m_worklist_sz;
			if (m_worklist_sz == 0)
				m_worklist_back = m_worklist_front;

			// push work_node into pool
			mutex_guard mg1(m_nodepool_lock);
			node->next = m_nodepool;
			m_nodepool = node;
			++m_nodepool_sz;

			if (node->work.type == work_t::type_fence)
				node->work.fence->_mark_done();
		}

		// return the work
		return work;
	}

	inline void work_queue::push_work(work_t work) {
		// get a node from the pool
		work_node* node = get_or_make_node();
		// attach work
		node->work = work;

		// push it into the list
		mutex_guard mg(m_worklist_lock);
		if (m_worklist_back) m_worklist_back->next = node;
		m_worklist_back = node;
		if (!m_worklist_front) m_worklist_front = m_worklist_back;
		m_worklist_back->next = nullptr;
		++m_worklist_sz;
	}

	inline work_queue::work_node* work_queue::get_or_make_node() {
		if (m_nodepool) {
			// get a node from the pool
			mutex_guard mg(m_nodepool_lock);
			work_node* node = m_nodepool;
			m_nodepool = node->next;
			--m_nodepool_sz;
			return node;
		}
		// make a new node as needed
		return new work_node();
	}

	inline work_queue::work_t work_queue::_get_work() {
		return pop_work();
	}

	inline void work_queue::_add_worker(worker_thread* worker) {
		mutex_guard mg(m_worker_lock);
		m_workers.push_back(worker);
	}

	inline void work_queue::_remove_worker(worker_thread* worker) {
		mutex_guard mg(m_worker_lock);
		for (auto it = m_workers.begin(); it != m_workers.end(); ++it) {
			if (*it == worker) {
				m_workers.erase(it);
				return;
			}
		}
	}

}