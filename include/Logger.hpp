/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asanni <asanni@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/05 14:17:10 by asanni            #+#    #+#             */
/*   Updated: 2025/07/31 12:01:13 by josfelip         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGGER_HPP
# define LOGGER_HPP

# include <iostream>
# include <string>


enum LogLevel {
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARNING,
	LOG_ERROR,
	LOG_CRITICAL
};

# ifndef LOG_LEVEL
# define LOG_LEVEL LOG_INFO
# endif

class Logger {
private:
	LogLevel _filterLevel;

public:
	Logger();
	Logger(LogLevel level);
	Logger(const Logger& other);
	Logger& operator=(const Logger& other);
	~Logger();

	void setFilterLevel(LogLevel level);
	LogLevel getFilterLevel() const;

	void log(LogLevel level, const std::string& message) const;
	void debug(const std::string& message) const;
	void info(const std::string& message) const;
	void warning(const std::string& message) const;
	void error(const std::string& message) const;
	void critical(const std::string& message) const;
};

#endif
