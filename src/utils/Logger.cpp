/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asanni <asanni@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 17:23:07 by asanni            #+#    #+#             */
/*   Updated: 2025/07/26 15:52:00 by asanni           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include <iostream>
#include <cctype>

Logger::Logger(void) : _level(LOG_INFO) {}

Logger::Logger(const Logger& other) : _level(other._level) {}

Logger& Logger::operator=(const Logger& other) {
	if (this != &other)
		_level = other._level;
	return *this;
}

Logger::~Logger(void) {}

void Logger::setLevel(std::string level) {
	level = toLower(level);
	if (level == "critical")
		_level = LOG_CRITICAL;
	else if(level == "error")
		_level = LOG_ERROR;
	else if(level == "warning")
		_level = LOG_WARNING;
	else if(level == "info")
		_level = LOG_INFO;
	else if(level == "debug")
		_level = LOG_DEBUG;
}

LogLevel Logger::getLevel(void) const {
	return _level;
}

void Logger::log(LogLevel level, const std::string& message) const {
	if (level < _level)
		return;

	switch (level) {
		case LOG_CRITICAL: critical(message); break;
		case LOG_ERROR:    error(message);    break;
		case LOG_WARNING:  warning(message);  break;
		case LOG_INFO:     info(message);     break;
		case LOG_DEBUG:    debug(message);    break;
	}
}

void Logger::critical(const std::string& message) const {
	std::cerr << "[CRITICAL] " << message << std::endl;
}

void Logger::error(const std::string& message) const {
	std::cerr << "[ERROR] " << message << std::endl;
}

void Logger::warning(const std::string& message) const {
	std::cout << "[WARNING] " << message << std::endl;
}

void Logger::info(const std::string& message) const {
	std::cout << "[INFO] " << message << std::endl;
}

void Logger::debug(const std::string& message) const {
	std::cout << "[DEBUG] " << message << std::endl;
}

std::string Logger::toLower(std::string level) {
	for (std::string::size_type i = 0; i < level.length(); ++i) {
		level[i] = std::tolower(level[i]);
	}
}

void Logger::error(const std::string& message) const {
	if (_level >= LOG_ERROR)
		log(LOG_ERROR, message);
}

void Logger::info(const std::string& message) const {
	if (_level >= LOG_INFO)
		log(LOG_INFO, message);
}

void Logger::debug(const std::string& message) const {
	if (_level >= LOG_DEBUG)
		log(LOG_DEBUG, message);
}
