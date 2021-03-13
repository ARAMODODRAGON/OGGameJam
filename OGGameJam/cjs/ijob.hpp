#ifndef CJS_IJOB_HPP
#define CJS_IJOB_HPP
#include "common.hpp"

namespace cjs {

	// base class for all jobs
	struct ijob {
		virtual void execute() = 0;
	};

}

#endif // CJS_!IJOB_HPP