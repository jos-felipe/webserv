/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asanni <asanni@student.42sp.org.br>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 17:23:07 by asanni            #+#    #+#             */
/*   Updated: 2025/07/15 19:32:40 by asanni           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include <iostream>

Logger::Logger(void){}

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
	switch (level) {
		case CRITICAL:
			std::cerr << "[ERROR] " << message << std::endl;
			break;
		case ERROR:
			std::cerr << "[ERROR] " << message << std::endl;
			break;
		case WARNING:
			std::cout << "[INFO] " << message << std::endl;
			break;
		case INFO:
			std::cout << "[INFO] " << message << std::endl;
			break;
		case DEBUG:
			std::cout << "[DEBUG] " << message << std::endl;
			break;
	}
}

void Logger::error(const std::string& message) const {
	if (_level == LOG_ERROR)
		log(LOG_ERROR, message);
}

void Logger::info(const std::string& message) const {
	if (_level == LOG_INFO)
		log(LOG_INFO, message);
}

void Logger::debug(const std::string& message) const {
	if (_level == LOG_DEBUG)
		log(LOG_DEBUG, message);
}
