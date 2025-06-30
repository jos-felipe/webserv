/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asanni <asanni@student.42sp.org.br>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 17:12:57 by asanni            #+#    #+#             */
/*   Updated: 2025/06/30 17:39:14 by asanni           ###   ########.fr       */
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
};

extern Logger g_logger;

# define LOG_ERROR(logger, msg) if ((logger).getLevel() >= LOG_ERROR) (logger).log(LOG_ERROR, msg)
# define LOG_INFO(logger, msg)  if ((logger).getLevel() >= LOG_INFO)  (logger).log(LOG_INFO, msg)
# define LOG_DEBUG(logger, msg) if ((logger).getLevel() >= LOG_DEBUG) (logger).log(LOG_DEBUG, msg)

#endif