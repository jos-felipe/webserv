/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asanni <asanni@student.42sp.org.br>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/26 13:10:12 by josfelip          #+#    #+#             */
/*   Updated: 2025/08/05 18:12:45 by asanni           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <string>
# include <vector>
# include <map>
# include <set>
# include "Logger.hpp"

/**
 * @struct LocationConfig
 * @brief Configuration for a specific route/location
 */
struct LocationConfig
{
	std::string					path;
	std::string					root;
	std::string					index;
	std::set<std::string>		allowedMethods;
	bool						autoindex;
	std::string					redirect;
	std::string					uploadStore;
	std::string					cgiPath;
	std::set<std::string>		cgiExtensions;

	LocationConfig() : autoindex(false) {}
};

/**
 * @struct ServerConfig
 * @brief Configuration for a virtual server
 */
struct ServerConfig
{
	std::string							host;
	int									port;
	std::vector<std::string>			serverNames;
	std::map<int, std::string>			errorPages;
	unsigned long						clientMaxBodySize;
	std::vector<LocationConfig>			locations;

	ServerConfig() : port(80), clientMaxBodySize(1048576) {}
};

/**
 * @class Config
 * @brief Parses and stores server configuration
 * 
 * This class is responsible for parsing the configuration file and 
 * providing access to the server configuration settings.
 */
class Config
{
private:
	std::string					_configPath;
	std::vector<ServerConfig>	_servers;
	Logger									_logger;
	
	/**
	 * Parse the configuration file
	 */
	void						parseConfig(void);
	
	/**
	 * Parse a server block from the configuration
	 */
	ServerConfig				parseServerBlock(std::vector<std::string>& lines, 
										size_t& index);
	
	/**
	 * Parse a location block from the configuration
	 */
	LocationConfig				parseLocationBlock(std::vector<std::string>& lines,
										size_t& index);
	
	/**
	 * Validate the configurations
	 */
	void						validateConfig(void);
	
	/**
	 * Tokenize a configuration line
	 */
	std::vector<std::string>	tokenizeLine(const std::string& line);

public:
	/**
	 * Default constructor
	 */
	Config(void);
	
	/**
	 * Constructor with path to configuration file
	 */
	Config(const std::string& configPath);
	
	/**
	 * Copy constructor
	 */
	Config(const Config& other);
	
	/**
	 * Destructor
	 */
	~Config(void);
	
	/**
	 * Assignment operator
	 */
	Config&			operator=(const Config& other);
	
	/**
	 * Get all configured servers
	 */
	const std::vector<ServerConfig>&	getServers(void) const;
	
	/**
	 * Find server configuration by host and port
	 */
	const ServerConfig*				findServer(const std::string& host, 
									int port, const std::string& serverName) const;
	
	/**
	 * Get default error page for a status code
	 */
	std::string						getDefaultErrorPage(int statusCode) const;
};

#endif