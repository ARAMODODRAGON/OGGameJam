#include "Debugger.hpp"
#include <iostream>
#include <fstream>

#define LOG_FILE_PATH string("Logs/Log_")

static const string& GetLogPath() {
	static string logFilepath = "";
	if (logFilepath == "") {
		// find a unique file name to use
		size_t i = 0;
		logFilepath = LOG_FILE_PATH + std::to_string(i);
		std::fstream file(logFilepath);
		while (file.is_open()) {
			// try to open a file with name:
			file.close();
			++i;
			logFilepath = LOG_FILE_PATH + std::to_string(i);
			file.open(logFilepath);
		}
		file.close();
	}
	return logFilepath;
}

void Debugger::Log(const string& msg, const string& file, size_t line) {

	// open file at the end
	std::ofstream filestream;
	filestream.open(GetLogPath(), std::ios::app);

	// add msg
	filestream << "[Log(ln:" << line << ")]: " << msg << std::endl << "\t[file: " << file << "]" << std::endl;
	//#if _DEBUG
	std::cout << "[Log]: " << msg << std::endl;
	//#endif

	// close
	filestream.close();
}

void Debugger::Trace(const string& msg, const string& file, size_t line) {

	// open file at the end
	std::ofstream filestream;
	filestream.open(GetLogPath(), std::ios::app);

	// add msg
	filestream << "[Trace(ln:" << line << ")]: " << msg << std::endl << "\t[file: " << file << "]" << std::endl;
	//#if _DEBUG
	std::cout << "[Trace]: " << msg << std::endl;
	//#endif

	// close
	filestream.close();
}

void Debugger::Warning(const string& msg, const string& file, size_t line) {

	// open file at the end
	std::ofstream filestream;
	filestream.open(GetLogPath(), std::ios::app);

	// add msg
	filestream << "[Warning(ln:" << line << ")]: " << msg << std::endl << "\t[file: " << file << "]" << std::endl;
	//#if _DEBUG
	std::cout << "[Warning]: " << msg << std::endl;
	//#endif

	// close
	filestream.close();
}

void Debugger::Error(const string& msg, const string& file, size_t line) {

	// open file at the end
	std::ofstream filestream;
	filestream.open(GetLogPath(), std::ios::app);

	// add msg
	filestream << "[Error(ln:" << line << ")]: " << msg << std::endl << "\t[file: " << file << "]" << std::endl;
	//#if _DEBUG
	std::cout << "[Error]: " << msg << std::endl;
	//#endif

	// close
	filestream.close();
}

void Debugger::FatalError(const string& msg, const string& file, size_t line) {

	// open file at the end
	std::ofstream filestream;
	filestream.open(GetLogPath(), std::ios::app);

	// add msg
	filestream << "[FatalError(ln:" << line << ")]: " << msg << std::endl << "\t[file: " << file << "]" << std::endl;
	//#if _DEBUG
	std::cout << "[FatalError]: " << msg << std::endl;
	//#endif

	// close
	filestream.close();
}

