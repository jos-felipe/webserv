/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asanni <asanni@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/05 14:17:10 by asanni            #+#    #+#             */
/*   Updated: 2025/07/19 16:47:50 by asanni           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGGER_HPP
# define LOGGER_HPP

# include <iostream>
# include <string>



# ifndef LOG_LEVEL
# define LOG_LEVEL INFO
# endif

enum LogLevel {
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARNING,
	LOG_ERROR,
	LOG_CRITICAL
};

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

	void log(const std::string& message) const;

	// Métodos convenientes por nível:
	void debug(const std::string& message) const;
	void info(const std::string& message) const;
	void warning(const std::string& message) const;
	void error(const std::string& message) const;
	void critical(const std::string& message) const;
};

#endif
