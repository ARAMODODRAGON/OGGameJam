
namespace cjs {

	inline fence::fence()
		: m_shouldawait(false), m_done(false), m_shouldresume(false), m_joinedcount(0) { }

	inline fence::~fence() {
		await_and_resume();
	}

	inline void fence::await() {
		while (m_shouldawait && !m_done);
		m_shouldawait = false;
	}

	inline void fence::resume() {
		m_shouldresume = true;
		while (m_joinedcount > 0);
	}

	inline void fence::await_and_resume() {
		await();
		resume();
	}

	inline void fence::_submit() {
		await_and_resume();
		m_done = m_shouldresume = false;
		m_shouldawait = true;
	}

	inline void fence::_join() {
		++m_joinedcount;
		while (!m_shouldresume);
		--m_joinedcount;
	}

	inline void fence::_mark_done() {
		m_done = true;
	}
}