/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asanni <asanni@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 17:23:07 by asanni            #+#    #+#             */
/*   Updated: 2025/07/31 12:39:16 by josfelip         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"

Logger::Logger() : _filterLevel(LOG_LEVEL) {}

Logger::Logger(LogLevel level) : _filterLevel(level) {}

Logger::Logger(const Logger& other) {
	*this = other;
}

Logger& Logger::operator=(const Logger& other) {
	if (this != &other) {
		_filterLevel = other._filterLevel;
	}
	return *this;
}

Logger::~Logger() {}

void Logger::setFilterLevel(LogLevel level) {
	_filterLevel = level;
}

LogLevel Logger::getFilterLevel() const {
	return _filterLevel;
}

void Logger::log(LogLevel level, const std::string& message) const {
	if (level >= _filterLevel) {
		if (level == LOG_DEBUG)
			std::cout << "[DEBUG] " << message << std::endl;
		else if (level == LOG_INFO)
			std::cout << "[INFO] " << message << std::endl;
		else if (level == LOG_WARNING)
			std::cout << "[WARNING] " << message << std::endl;
		else if (level == LOG_ERROR)
			std::cerr << "[ERROR] " << message << std::endl;
		else if (level == LOG_CRITICAL)
			std::cerr << "[CRITICAL] " << message << std::endl;
	}
}
