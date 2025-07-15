/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asanni <asanni@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/05 14:17:10 by asanni            #+#    #+#             */
/*   Updated: 2025/07/05 14:17:14 by asanni           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGGER_HPP
# define LOGGER_HPP

# include <iostream>
# include <string>

enum LogLevel {
	LOG_ERROR,
	LOG_INFO,
	LOG_DEBUG
};

class Logger {
private:
	LogLevel _level;

public:
	Logger(void);
	Logger(const Logger& other);
	Logger& operator=(const Logger& other);
	~Logger(void);

	void setLevel(LogLevel level);
	LogLevel getLevel(void) const;

	void log(LogLevel level, const std::string& message) const;
std::ostringstream oss;
	// Métodos convenientes por nível:
	void error(const std::string& message) const;
	void info(const std::string& message) const;
	void debug(const std::string& message) const;
};

#endif
