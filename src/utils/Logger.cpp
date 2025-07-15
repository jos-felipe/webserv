
#include "Logger.hpp"
#include <iostream>

Logger g_logger;

Logger::Logger(void) : _level(LOG_LEVEL) {}

Logger::Logger(const Logger& other) : _level(other._level) {}

Logger& Logger::operator=(const Logger& other) {
	if (this != &other)
		_level = other._level;
	return *this;
}

Logger::~Logger(void) {}

void Logger::setLevel(LogLevel level) {
	_level = level;
}

LogLevel Logger::getLevel(void) const {
	return _level;
}

void Logger::log(LogLevel level, const std::string& message) const {

	if (level == _level) {
		switch (level) {
            case LOG_ERROR:
		        std::cerr << "[ERROR] " << message << std::endl;
		        break;
		    case LOG_INFO:
		        std::cout << "[INFO] " << message << std::endl;
		        break;
		    case LOG_DEBUG:
		        std::cout << "[DEBUG] " << message << std::endl;
		        break;
        }
	}

    oss.str("");
	oss.clear();    
}
