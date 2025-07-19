/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asanni <asanni@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/13 10:00:00 by josfelip          #+#    #+#             */
/*   Updated: 2025/07/19 17:07:27 by asanni           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiHandler.hpp"
#include "HttpRequest.hpp"
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>

/**
 * Default constructor
 */
CgiHandler::CgiHandler(void)
{
}

/**
 * Constructor initializes server with Logger
 */
CgiHandler::CgiHandler(Logger logger): _logger(logger)
{
}

/**
 * Copy constructor
 */
CgiHandler::CgiHandler(const CgiHandler& other) :
	_scriptPath(other._scriptPath),
	_interpreter(other._interpreter),
	_pathInfo(other._pathInfo),
	_queryString(other._queryString),
	_requestMethod(other._requestMethod),
	_contentType(other._contentType),
	_contentLength(other._contentLength),
	_requestBody(other._requestBody),
	_workingDirectory(other._workingDirectory),
	_envVars(other._envVars),
	_logger(other._logger)
{
}

/**
 * Destructor
 */
CgiHandler::~CgiHandler(void)
{
}

/**
 * Assignment operator
 */
CgiHandler&	CgiHandler::operator=(const CgiHandler& other)
{
	if (this != &other)
	{
		_scriptPath = other._scriptPath;
		_interpreter = other._interpreter;
		_pathInfo = other._pathInfo;
		_queryString = other._queryString;
		_requestMethod = other._requestMethod;
		_contentType = other._contentType;
		_contentLength = other._contentLength;
		_requestBody = other._requestBody;
		_workingDirectory = other._workingDirectory;
		_envVars = other._envVars;
	}
	return *this;
}

/**
 * Check if a file should be handled by CGI based on extension
 */
bool	CgiHandler::isCgiFile(const std::string& path, const LocationConfig& location)
{
	if (location.cgiExtensions.empty())
		return false;
	
	size_t dotPos = path.find_last_of('.');
	if (dotPos == std::string::npos)
		return false;
	
	std::string extension = path.substr(dotPos);
	return location.cgiExtensions.find(extension) != location.cgiExtensions.end();
}

/**
 * Execute CGI script and generate HTTP response
 */
HttpResponse	CgiHandler::handleCgiRequest(const HttpRequest& request,
	const LocationConfig& location, const std::string& scriptPath)
{
	std::ostringstream oss1;
	oss1 << "DEBUG: Handling CGI request for " << scriptPath << std::endl;
	_logger.log(LOG_DEBUG, oss1.str());
	
	HttpResponse response;
	
	// Check if script exists and is executable
	struct stat statBuf;
	if (stat(scriptPath.c_str(), &statBuf) != 0)
	{
		std::ostringstream oss2;
		oss2 << "DEBUG: CGI script not found: " << scriptPath << std::endl;
		_logger.log(LOG_DEBUG, oss2.str());
		response.setStatus(404);
		response.setBody("CGI script not found");
		return response;
	}
	
	if (!(statBuf.st_mode & S_IXUSR))
	{
		std::ostringstream oss3;
		oss3 << "DEBUG: CGI script not executable: " << scriptPath << std::endl;
		_logger.log(LOG_DEBUG, oss3.str());
		response.setStatus(403);
		response.setBody("CGI script not executable");
		return response;
	}
	
	// Set up CGI handler properties
	_scriptPath = scriptPath;
	
	// Convert to absolute path if relative
	if (_scriptPath[0] != '/')
	{
		char* cwd = getcwd(NULL, 0);
		if (cwd)
		{
			_scriptPath = std::string(cwd) + "/" + _scriptPath;
			free(cwd);
		}
	}
	
	// Extract file extension and get interpreter
	size_t dotPos = scriptPath.find_last_of('.');
	if (dotPos != std::string::npos)
	{
		std::string extension = scriptPath.substr(dotPos);
		_interpreter = getInterpreter(extension, location);
	}
	
	// Set working directory to script directory
	size_t lastSlash = _scriptPath.find_last_of('/');
	if (lastSlash != std::string::npos)
	{
		_workingDirectory = _scriptPath.substr(0, lastSlash);
	}
	else
	{
		_workingDirectory = ".";
	}
	
	// Setup CGI environment variables
	setupEnvironment(request, location);
	
	// Execute CGI script
	std::string cgiOutput = executeCgi();
	
	if (cgiOutput.empty())
	{
		std::ostringstream oss4;
		oss4 << "DEBUG: CGI execution failed" << std::endl;
		_logger.log(LOG_DEBUG, oss4.str());
		response.setStatus(500);
		response.setBody("Internal Server Error: CGI execution failed");
		return response;
	}
	
	// Parse CGI output and set response
	parseCgiOutput(cgiOutput, response);
	
	return response;
}

/**
 * Setup CGI environment variables according to CGI/1.1 specification
 */
void	CgiHandler::setupEnvironment(const HttpRequest& request, const LocationConfig& location)
{
	(void)location; // May be used for additional configuration
	
	// Extract PATH_INFO from request URI
	std::string uri = request.getUri();
	size_t queryPos = uri.find('?');
	std::string path = (queryPos != std::string::npos) ? uri.substr(0, queryPos) : uri;
	
	// PATH_INFO is the full path as specified in requirements
	_pathInfo = path;
	_queryString = (queryPos != std::string::npos) ? uri.substr(queryPos + 1) : "";
	_requestMethod = request.getMethod();
	_contentType = request.getHeader("Content-Type");
	_requestBody = request.getBody();
	
	std::ostringstream oss;
	oss << _requestBody.length();
	_contentLength = oss.str();
	
	// Set up standard CGI environment variables
	_envVars["REQUEST_METHOD"] = _requestMethod;
	_envVars["PATH_INFO"] = _pathInfo;
	_envVars["QUERY_STRING"] = _queryString;
	_envVars["CONTENT_TYPE"] = _contentType;
	_envVars["CONTENT_LENGTH"] = _contentLength;
	_envVars["SCRIPT_NAME"] = _pathInfo; // Use the request path, not the full script path
	_envVars["SCRIPT_FILENAME"] = _scriptPath; // Full path to script file
	_envVars["SERVER_NAME"] = "webserv";
	_envVars["SERVER_PORT"] = "8080";
	_envVars["SERVER_PROTOCOL"] = "HTTP/1.1";
	_envVars["GATEWAY_INTERFACE"] = "CGI/1.1";
	_envVars["REDIRECT_STATUS"] = "200"; // Required by PHP-CGI for security
	
	// Add all HTTP headers as HTTP_* environment variables
	const std::map<std::string, std::string>& headers = request.getHeaders();
	for (std::map<std::string, std::string>::const_iterator it = headers.begin();
		 it != headers.end(); ++it)
	{
		std::string envName = "HTTP_" + it->first;
		// Convert to uppercase and replace - with _
		for (size_t i = 0; i < envName.length(); ++i)
		{
			if (envName[i] == '-')
				envName[i] = '_';
			else
				envName[i] = std::toupper(envName[i]);
		}
		_envVars[envName] = it->second;
	}
	
	std::ostringstream oss1;
	oss1 << "DEBUG: CGI environment variables set up:" << std::endl;
	_logger.log(LOG_DEBUG, oss1.str());
	for (std::map<std::string, std::string>::const_iterator it = _envVars.begin();
		 it != _envVars.end(); ++it)
	{
		std::cout << "  " << it->first << "=" << it->second << std::endl;
	}
}

/**
 * Execute the CGI script using fork/exec
 */
std::string	CgiHandler::executeCgi(void)
{
	int inputPipe[2];
	int outputPipe[2];
	
	// Create pipes for communication with CGI process
	if (pipe(inputPipe) == -1 || pipe(outputPipe) == -1)
	{
		std::ostringstream oss1;
		oss1 << "DEBUG: Failed to create pipes: " << strerror(errno) << std::endl;
		_logger.log(LOG_ERROR, oss1.str());
		return "";
	}
	
	pid_t pid = fork();
	
	if (pid == -1)
	{
		std::ostringstream oss2;
		oss2 << "DEBUG: Fork failed: " << strerror(errno) << std::endl;
		_logger.log(LOG_ERROR, oss2.str());
		close(inputPipe[0]);
		close(inputPipe[1]);
		close(outputPipe[0]);
		close(outputPipe[1]);
		return "";
	}
	
	if (pid == 0)
	{
		// Child process - execute CGI script
		
		// Close unused pipe ends
		close(inputPipe[1]);
		close(outputPipe[0]);
		
		// Redirect stdin and stdout
		if (dup2(inputPipe[0], STDIN_FILENO) == -1 ||
			dup2(outputPipe[1], STDOUT_FILENO) == -1)
		{
			std::ostringstream oss3;
			oss3 << "DEBUG: dup2 failed: " << strerror(errno) << std::endl;
			_logger.log(LOG_ERROR, oss3.str());
			exit(1);
		}
		
		// Close original pipe file descriptors
		close(inputPipe[0]);
		close(outputPipe[1]);
		
		// Change working directory
		if (chdir(_workingDirectory.c_str()) == -1)
		{
			std::ostringstream oss4;
			oss4 << "DEBUG: chdir failed: " << strerror(errno) << std::endl;
			_logger.log(LOG_ERROR, oss4.str());
			exit(1);
		}
		
		// Set up environment
		char** envArray = createEnvArray();
		
		// Execute CGI script
		char* args[4];
		if (!_interpreter.empty())
		{
			// Use interpreter 
			if (_interpreter.find("php") != std::string::npos)
			{
				// For php-cgi, we don't pass the script as argument, it uses SCRIPT_FILENAME
				args[0] = const_cast<char*>(_interpreter.c_str());
				args[1] = NULL;
			}
			else
			{
				// For python and other interpreters, pass script as argument
				args[0] = const_cast<char*>(_interpreter.c_str());
				args[1] = const_cast<char*>(_scriptPath.c_str());
				args[2] = NULL;
			}
			execve(_interpreter.c_str(), args, envArray);
		}
		else
		{
			// Execute script directly
			args[0] = const_cast<char*>(_scriptPath.c_str());
			args[1] = NULL;
			execve(_scriptPath.c_str(), args, envArray);
		}
		
		// If we reach here, execve failed
		std::ostringstream oss5;
		oss5 << "DEBUG: execve failed: " << strerror(errno) << std::endl;
		_logger.log(LOG_ERROR, oss5.str());
		freeEnvArray(envArray);
		exit(1);
	}
	else
	{
		// Parent process - communicate with CGI
		
		// Close unused pipe ends
		close(inputPipe[0]);
		close(outputPipe[1]);
		
		// Send request body to CGI if present
		if (!_requestBody.empty())
		{
			ssize_t bytesWritten = write(inputPipe[1], _requestBody.c_str(), _requestBody.length());
			if (bytesWritten == -1)
			{
				std::ostringstream oss5;
				oss5 << "DEBUG: Failed to write to CGI stdin: " << strerror(errno) << std::endl;
				_logger.log(LOG_ERROR, oss5.str());
			}
		}
		close(inputPipe[1]); // Signal EOF to CGI
		
		// Read CGI output
		std::string output;
		char buffer[4096];
		ssize_t bytesRead;
		
		while ((bytesRead = read(outputPipe[0], buffer, sizeof(buffer))) > 0)
		{
			output.append(buffer, bytesRead);
		}
		
		close(outputPipe[0]);
		
		// Wait for child process to complete
		int status;
		waitpid(pid, &status, 0);
		
		if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
		{
			std::ostringstream oss6;
			oss6 << "DEBUG: CGI script exited with status " << WEXITSTATUS(status) << std::endl;
			_logger.log(LOG_DEBUG, oss6.str());
		}
		std::ostringstream oss7;
		oss7 << "DEBUG: CGI output (" << output.length() << " bytes):" << output << std::endl;
		_logger.log(LOG_DEBUG, oss7.str());
		
		return output;
	}
}

/**
 * Parse CGI output to extract headers and body
 */
void	CgiHandler::parseCgiOutput(const std::string& output, HttpResponse& response)
{
	size_t headerEndPos = output.find("\r\n\r\n");
	if (headerEndPos == std::string::npos)
	{
		headerEndPos = output.find("\n\n");
		if (headerEndPos == std::string::npos)
		{
			// No headers found, treat entire output as body
			response.setStatus(200);
			response.setBody(output);
			response.addHeader("Content-Type", "text/html");
			return;
		}
		headerEndPos += 2;
	}
	else
	{
		headerEndPos += 4;
	}
	
	// Parse headers
	std::string headerSection = output.substr(0, headerEndPos - 2);
	std::string body = output.substr(headerEndPos);
	
	std::istringstream headerStream(headerSection);
	std::string line;
	bool statusSet = false;
	
	while (std::getline(headerStream, line))
	{
		// Remove carriage return if present
		if (!line.empty() && line[line.length() - 1] == '\r')
			line = line.substr(0, line.length() - 1);
		
		if (line.empty())
			continue;
		
		size_t colonPos = line.find(':');
		if (colonPos == std::string::npos)
			continue;
		
		std::string headerName = line.substr(0, colonPos);
		std::string headerValue = line.substr(colonPos + 1);
		
		// Trim whitespace
		size_t start = headerValue.find_first_not_of(" \t");
		if (start != std::string::npos)
		{
			size_t end = headerValue.find_last_not_of(" \t");
			headerValue = headerValue.substr(start, end - start + 1);
		}
		
		// Handle special headers
		if (headerName == "Status" && !statusSet)
		{
			int statusCode = atoi(headerValue.c_str());
			if (statusCode > 0)
			{
				response.setStatus(statusCode);
				statusSet = true;
			}
		}
		else
		{
			response.addHeader(headerName, headerValue);
		}
		std::ostringstream oss;
		oss << "DEBUG: CGI header: " << headerName << ": " << headerValue << std::endl;
		_logger.log(LOG_DEBUG, oss.str());
	}
	
	// Set default status if not set by CGI
	if (!statusSet)
	{
		response.setStatus(200);
	}
	
	// Set body
	response.setBody(body);
	
	std::cout << "DEBUG: CGI response body (" << body.length() << " bytes)" << std::endl;
}

/**
 * Convert environment map to char** array for execve
 */
char**	CgiHandler::createEnvArray(void)
{
	char** envArray = new char*[_envVars.size() + 1];
	size_t i = 0;
	
	for (std::map<std::string, std::string>::const_iterator it = _envVars.begin();
		 it != _envVars.end(); ++it, ++i)
	{
		std::string envStr = it->first + "=" + it->second;
		envArray[i] = new char[envStr.length() + 1];
		strcpy(envArray[i], envStr.c_str());
	}
	
	envArray[i] = NULL;
	return envArray;
}

/**
 * Free the environment array created by createEnvArray
 */
void	CgiHandler::freeEnvArray(char** envArray)
{
	if (!envArray)
		return;
	
	for (size_t i = 0; envArray[i]; ++i)
	{
		delete[] envArray[i];
	}
	delete[] envArray;
}

/**
 * Get the interpreter path for a given file extension
 */
std::string	CgiHandler::getInterpreter(const std::string& extension, const LocationConfig& location)
{
	// Check if specific interpreter is configured for this location
	if (!location.cgiPath.empty())
	{
		return location.cgiPath;
	}
	
	// Default interpreters based on file extension
	if (extension == ".php")
	{
		return "/usr/bin/php-cgi";
	}
	else if (extension == ".py")
	{
		return "/usr/bin/python3";
	}
	else if (extension == ".pl")
	{
		return "/usr/bin/perl";
	}
	
	// No interpreter needed (script should be executable)
	return "";
}
