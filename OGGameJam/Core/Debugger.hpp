#ifndef _CORE_DEBUGGER_HPP
#define _CORE_DEBUGGER_HPP
#include <glm\glm.hpp>
#include <glm\gtx\string_cast.hpp>
#include <string>

// disables the copy constructor and operator
#define OGJ_NO_COPY(TYPE)				\
TYPE(const TYPE&) = delete;				\
TYPE& operator=(const TYPE&) = delete;		

// disables the move constructor and operator
#define OGJ_NO_MOVE(TYPE)				\
TYPE(TYPE&&) = delete;					\
TYPE& operator=(TYPE&&) = delete;

// disables copy/move constructors and operators
// and disables the default constructor and destructor
#define OGJ_NON_CONSTRUCTABLE(TYPE)		\
OGJ_NO_COPY(TYPE) OGJ_NO_MOVE(TYPE)		\
TYPE() = delete;						\
~TYPE() = delete;

using std::string;

class Debugger {
	OGJ_NON_CONSTRUCTABLE(Debugger);
public:

	static void Log(const string& msg, const string& file, size_t line);
	static void Trace(const string& msg, const string& file, size_t line);
	static void Warning(const string& msg, const string& file, size_t line);
	static void Error(const string& msg, const string& file, size_t line);
	static void FatalError(const string& msg, const string& file, size_t line);

};

namespace stringable {
	using std::to_string;
	using glm::to_string;
}

#define OGJ_DEBUG_LOG(msg) \
	::Debugger::Log(msg, __FILE__, __LINE__)
#define OGJ_DEBUG_TRACE(msg) \
	::Debugger::Trace(msg, __FILE__, __LINE__)
#define OGJ_DEBUG_WARNING(msg) \
	::Debugger::Warning(msg, __FILE__, __LINE__)
#define OGJ_DEBUG_ERROR(msg) \
	::Debugger::Error(msg, __FILE__, __LINE__)
#define OGJ_DEBUG_FATAL_ERROR(msg) \
	::Debugger::FatalError(msg, __FILE__, __LINE__)

#define VTOS(value) \
	::stringable::to_string(value)

#endif // !_CORE_DEBUGGER_HPP