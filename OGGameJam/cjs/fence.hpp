#ifndef CJS_FENCE_HPP
#define CJS_FENCE_HPP
#include <atomic>

namespace cjs {

	class ifence {
	public:
		virtual ~ifence() = 0 { }
		virtual void _submit() = 0;
		virtual void _join() = 0;
		virtual void _mark_done() = 0;
	};

	// can quickly stop and start worker threads
	// not very performance efficent 
	class fence : public ifence {
	public:

		fence();
		~fence() final;

		// waits for all threads to be blocked by this fence
		void await();

		// lets all threads resume running
		void resume();

		// await() then resume()
		void await_and_resume();

	private:

		std::atomic_bool m_shouldawait;
		std::atomic_bool m_done;
		std::atomic_bool m_shouldresume;
		std::atomic_size_t m_joinedcount;

		void _submit() override;
		void _join() override;
		void _mark_done() override;
	};

	// stops the threads but may not wake them immediately
	// saves performance by completely stopping the thread until its next needed
	//class slow_fence : public ifence {
	//public:
	//	
	//	slow_fence();
	//	~slow_fence() final;
	//
	//private:
	//	void _join() override;
	//	void _mark_done() override;
	//};

}

#include "fence.inl"

#endif // !CJS_FENCE_HPP