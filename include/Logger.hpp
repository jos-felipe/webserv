/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asanni <asanni@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/05 14:17:10 by asanni            #+#    #+#             */
/*   Updated: 2025/07/26 17:49:45 by asanni           ###   ########.fr       */
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
	LogLevel _level;
	std::string toLower(std::string level);

public:
	Logger(void);
	Logger(const Logger& other);
	Logger& operator=(const Logger& other);
	~Logger(void);

	void setLevel(std::string level);
	LogLevel getLevel(void) const;

	void log(LogLevel level, const std::string& message) const;

	// Métodos convenientes por nível:
	void error(const std::string& message) const;
	void info(const std::string& message) const;
	void debug(const std::string& message) const;
	void critical(const std::string& message) const;
	void warning(const std::string& message) const;
};

#endif
