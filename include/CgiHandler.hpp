/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asanni <asanni@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/13 10:00:00 by josfelip          #+#    #+#             */
/*   Updated: 2025/07/26 17:34:39 by asanni           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HANDLER_HPP
# define CGI_HANDLER_HPP

# include <string>
# include <map>
# include <vector>
# include "Config.hpp"
# include "HttpRequest.hpp"
# include "HttpResponse.hpp"

class HttpRequest;

/**
 * @class CgiHandler
 * @brief Handles CGI script execution with fork/exec
 * 
 * This class manages the execution of CGI scripts including:
 * - Setting up environment variables (PATH_INFO, QUERY_STRING, etc.)
 * - Managing process creation with fork()
 * - Handling input/output pipes for communication with CGI process
 * - Parsing CGI output and generating HTTP responses
 */
class CgiHandler
{
private:
	std::string							_scriptPath;
	std::string							_interpreter;
	std::string							_pathInfo;
	std::string							_queryString;
	std::string							_requestMethod;
	std::string							_contentType;
	std::string							_contentLength;
	std::string							_requestBody;
	std::string							_workingDirectory;
	std::map<std::string, std::string>	_envVars;
	Logger								_logger;
	
	/**
	 * Setup CGI environment variables according to CGI/1.1 specification
	 */
	void								setupEnvironment(const HttpRequest& request,
													const LocationConfig& location);
	
	/**
	 * Execute the CGI script using fork/exec
	 * Returns the CGI output or empty string on error
	 */
	std::string							executeCgi(void);
	
	/**
	 * Parse CGI output to extract headers and body
	 */
	void								parseCgiOutput(const std::string& output,
													HttpResponse& response);
	
	/**
	 * Convert environment map to char** array for execve
	 */
	char**								createEnvArray(void);
	
	/**
	 * Free the environment array created by createEnvArray
	 */
	void								freeEnvArray(char** envArray);
	
	/**
	 * Get the interpreter path for a given file extension
	 */
	std::string							getInterpreter(const std::string& extension,
													const LocationConfig& location);

public:
	/**
	 * Default constructor
	 */
	CgiHandler(void);
	
	/**
	 * Copy constructor
	 */
	CgiHandler(const CgiHandler& other);
	
	/**
	 * Destructor
	 */
	~CgiHandler(void);
	
	/**
	 * Assignment operator
	 */
	CgiHandler&			operator=(const CgiHandler& other);
	
	/**
	 * Check if a file should be handled by CGI based on extension
	 */
	static bool							isCgiFile(const std::string& path,
												const LocationConfig& location);
	
	/**
	 * Execute CGI script and generate HTTP response
	 */
	HttpResponse						handleCgiRequest(const HttpRequest& request,
													const LocationConfig& location,
													const std::string& scriptPath);
};

#endif
