/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asanni <asanni@student.42sp.org.br>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/26 13:20:34 by josfelip          #+#    #+#             */
/*   Updated: 2025/08/06 19:27:56 by asanni           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

/**
 * Default constructor
 */
Config::Config(void) : _configPath("")
{
}

/**
 * Constructor reads and parses the configuration file
 */
Config::Config(const std::string& configPath) : _configPath(configPath)
{
	parseConfig();
	validateConfig();
}

/**
 * Copy constructor
 */
Config::Config(const Config& other) : _configPath(other._configPath),
	_servers(other._servers)
{
}

/**
 * Destructor
 */
Config::~Config(void)
{
}

/**
 * Assignment operator
 */
Config&	Config::operator=(const Config& other)
{
	if (this != &other)
	{
		_configPath = other._configPath;
		_servers = other._servers;
	}
	return *this;
}

/**
 * Parse the configuration file into server configurations
 */
void	Config::parseConfig(void)
{
	std::ifstream file(_configPath.c_str());
	
	if (!file.is_open())
		throw std::runtime_error("Failed to open config file: " + _configPath);
	
	std::vector<std::string> lines;
	std::string line;
	
	// Read all lines from the file
	while (std::getline(file, line))
	{
		// Skip comments and empty lines
		size_t commentPos = line.find('#');
		if (commentPos != std::string::npos)
			line = line.substr(0, commentPos);
			
		// Remove leading and trailing whitespace
		size_t start = line.find_first_not_of(" \t");
		if (start == std::string::npos)
			continue;
			
		size_t end = line.find_last_not_of(" \t");
		line = line.substr(start, end - start + 1);
		
		if (!line.empty())
			lines.push_back(line);
	}
	
	file.close();
	
	// Parse all server blocks
	size_t index = 0;
	while (index < lines.size())
	{
		if (lines[index].find("server {") == 0)
		{
			index++;
			ServerConfig server = parseServerBlock(lines, index);
			_servers.push_back(server);
		}
		else
		{
			throw std::runtime_error("Expected 'server {' at line " 
				+ lines[index]);
		}
	}
	
	if (_servers.empty())
		throw std::runtime_error("No server configurations found");
}

/**
 * Parse a server block from the configuration
 */
ServerConfig	Config::parseServerBlock(std::vector<std::string>& lines, 
					size_t& index)
{
	ServerConfig server;
	
	while (index < lines.size() && lines[index] != "}")
	{
		std::vector<std::string> tokens = tokenizeLine(lines[index]);
		
		if (tokens.empty())
		{
			index++;
			continue;
		}
		
		if (tokens[0] == "listen" && tokens.size() >= 2)
		{
			std::string hostPort = tokens[1];
			size_t colonPos = hostPort.find(':');
			
			if (colonPos != std::string::npos)
			{
				server.host = hostPort.substr(0, colonPos);
				std::istringstream(hostPort.substr(colonPos + 1)) >> server.port;
			}
			else
			{
				// Default to all interfaces if only port specified
				server.host = "0.0.0.0";
				std::istringstream(hostPort) >> server.port;
			}
		}
		else if (tokens[0] == "server_name" && tokens.size() >= 2)
		{
			for (size_t i = 1; i < tokens.size(); i++)
				server.serverNames.push_back(tokens[i]);
		}
		else if (tokens[0] == "error_page" && tokens.size() >= 3)
		{
			for (size_t i = 1; i < tokens.size() - 1; i++)
			{
				int code;
				std::istringstream(tokens[i]) >> code;
				server.errorPages[code] = tokens[tokens.size() - 1];
			}
		}
		else if (tokens[0] == "client_max_body_size" && tokens.size() >= 2)
		{
			std::string size = tokens[1];
			unsigned long value;
			std::istringstream(size) >> value;
			
			char unit = size[size.length() - 1];
			if (unit == 'K')
				value *= 1024;
			else if (unit == 'M')
				value *= 1024 * 1024;
			else if (unit == 'G')
				value *= 1024 * 1024 * 1024;
				
			server.clientMaxBodySize = value;
		}
		else if (tokens[0] == "location" && tokens.size() >= 3 
			&& tokens[2] == "{")
		{
			index++;
			LocationConfig location = parseLocationBlock(lines, index);
			location.path = tokens[1];
			server.locations.push_back(location);
			continue; // Skip the index increment at the end
		}
		
		index++;
	}
	
	// Move past the closing brace
	if (index < lines.size())
		index++;
		
	return server;
}

/**
 * Parse a location block from the configuration
 */
LocationConfig	Config::parseLocationBlock(std::vector<std::string>& lines, 
				size_t& index)
{
	LocationConfig location;
	
	while (index < lines.size() && lines[index] != "}")
	{
		std::vector<std::string> tokens = tokenizeLine(lines[index]);
		
		if (tokens.empty())
		{
			index++;
			continue;
		}
		
		if (tokens[0] == "root" && tokens.size() >= 2)
			location.root = tokens[1];
		else if (tokens[0] == "index" && tokens.size() >= 2)
			location.index = tokens[1];
		else if (tokens[0] == "method" && tokens.size() >= 2)
		{
			for (size_t i = 1; i < tokens.size(); i++)
				location.allowedMethods.insert(tokens[i]);
		}
		else if (tokens[0] == "autoindex" && tokens.size() >= 2)
			location.autoindex = (tokens[1] == "on");
		else if (tokens[0] == "return" && tokens.size() >= 2)
			location.redirect = tokens[1];
		else if (tokens[0] == "upload_store" && tokens.size() >= 2)
			location.uploadStore = tokens[1];
		else if (tokens[0] == "cgi_pass" && tokens.size() >= 2)
			location.cgiPath = tokens[1];
		else if (tokens[0] == "cgi_ext" && tokens.size() >= 2)
		{
			for (size_t i = 1; i < tokens.size(); i++)
				location.cgiExtensions.insert(tokens[i]);
		}
		
		index++;
	}
	
	// Move past the closing brace
	if (index < lines.size())
		index++;
		
	return location;
}

/**
 * Validate the parsed configuration for consistency
 */
void	Config::validateConfig(void)
{
	if (_servers.empty())
		throw std::runtime_error("No server configurations found");
		
	// Check each server
	for (size_t i = 0; i < _servers.size(); i++)
	{
		ServerConfig& server = _servers[i];
		
		// Ensure port is valid
		if (server.port <= 0 || server.port > 65535)
			throw std::runtime_error("Invalid port number");
			
		// Ensure locations have necessary settings
		for (size_t j = 0; j < server.locations.size(); j++)
		{
			LocationConfig& location = server.locations[j];
			
			// Root must be specified
			if (location.root.empty())
				throw std::runtime_error("Root not specified for location " 
					+ location.path);
			
			// If cgi_pass is specified, must have cgi_extensions
			if (!location.cgiPath.empty() && location.cgiExtensions.empty())
				throw std::runtime_error("CGI extensions not specified for location " 
					+ location.path);
		}
	}
}

/**
 * Tokenize a configuration line
 */
std::vector<std::string>	Config::tokenizeLine(const std::string& line)
{
	std::vector<std::string> tokens;
	std::istringstream iss(line);
	std::string token;
	
	while (iss >> token)
	{
		// Remove semicolon if present
		if (!token.empty() && token[token.length() - 1] == ';')
			token = token.substr(0, token.length() - 1);
			
		tokens.push_back(token);
	}
	
	return tokens;
}

/**
 * Get all configured servers
 */
const std::vector<ServerConfig>&	Config::getServers(void) const
{
	return _servers;
}

/**
 * Find server configuration by host, port and server name
 */
const ServerConfig* Config::findServer(const std::string& host, int port, 
	const std::string& serverName) const
{
	_logger.tempOss << "Finding server for host='" << host 
	<< "', port=" << port << ", name='" << serverName << "'";
	_logger.debug();

	const ServerConfig* defaultServer = NULL;

	// Log all available servers for debugging
	_logger.tempOss << "Total servers in config: " << _servers.size();

	for (size_t i = 0; i < _servers.size(); i++)
	{
	const ServerConfig& server = _servers[i];

	_logger.tempOss << "Checking server #" << i << ": host='" << server.host 
	<< "', port=" << server.port;
	_logger.debug();

	if (!server.serverNames.empty()) {
	_logger.tempOss << ", names=[";
	for (size_t j = 0; j < server.serverNames.size(); j++) {
	if (j > 0) _logger.tempOss << ", ";
	_logger.tempOss << "'" << server.serverNames[j] << "'";
	}
	_logger.tempOss << "]";
	}
	_logger.info();

	// Check for matching host & port
	bool hostMatches = (server.host == host || server.host == "0.0.0.0");
	bool portMatches = (server.port == port);

	if (hostMatches && portMatches)
	{
		_logger.tempOss << "Found matching host/port";
		_logger.debug();

	// Check if server name matches
	if (server.serverNames.empty())
	{
		_logger.tempOss << "Server has no server_names, using as default";
		_logger.debug();
	if (!defaultServer)
	defaultServer = &server;
	}
	else
	{
	for (size_t j = 0; j < server.serverNames.size(); j++)
	{
		_logger.tempOss << "Comparing server_name '" 
		<< server.serverNames[j] << "' with '" 
		<< serverName << "'";
		_logger.debug();

	if (server.serverNames[j] == serverName)
	{
		_logger.tempOss << "Server name matches!";
		_logger.debug();
		return &server;
	}
	}

	// No matching server name, but host/port match, so potential default
	if (!defaultServer)
	{
		_logger.tempOss << "No matching server_name, " 
		<< "but using as potential default";
		_logger.debug();
	defaultServer = &server;
	}
	}
	}
	}

	if (defaultServer){
		_logger.tempOss << "Returning default server for this host/port";
		_logger.debug();
	}

	else{
		_logger.tempOss << "No matching server found!";
		_logger.debug();
	}
	

	return defaultServer;
}

/**
 * Get default error page for status code
 */
std::string Config::getDefaultErrorPage(int statusCode) const
{
    // percorre todos os servidores
    for (size_t i = 0; i < _servers.size(); ++i)
    {
        const ServerConfig& server = _servers[i];

        //existe error_page explícito p/ este código?
        std::map<int, std::string>::const_iterator it = server.errorPages.find(statusCode);
        if (it == server.errorPages.end())
            continue;                

        const std::string& confPath = it->second;     

        //descobre qual root utilizar (pega o location "/")
        std::string rootDir;
        for (size_t j = 0; j < server.locations.size(); ++j)
        {
            if (server.locations[j].path == "/") {
                rootDir = server.locations[j].root;
                break;
            }
        }
        if (rootDir.empty())
            rootDir = ".";
						
        std::string fullPath;
        if (confPath.size() && confPath[0] == '/')     
            fullPath = rootDir + confPath;
        else
            fullPath = confPath;

        //tenta abrir o arquivo
        std::ifstream file(fullPath.c_str(), std::ios::in | std::ios::binary);
        if (file.is_open())
        {
            std::ostringstream ss;
            ss << file.rdbuf();
            return ss.str();
        }
        _logger.tempOss << "could not open error page'"
                  << fullPath  << ")\n";
									_logger.warning();
        
    }
    // se falhar, continua para a error page embutida
    std::ostringstream oss;
    oss << "<html><head><title>Error " << statusCode << "</title></head>"
        << "<body><h1>Error " << statusCode << "</h1></body></html>";
    return oss.str();
}
