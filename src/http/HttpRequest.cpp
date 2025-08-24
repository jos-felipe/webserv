/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asanni <asanni@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/26 13:50:42 by josfelip          #+#    #+#             */
/*   Updated: 2025/08/24 15:33:30 by asanni           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"
#include "CgiHandler.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <cerrno>
#include <fstream>
#include <ctime>
#include <dirent.h>
#include <sys/stat.h>

/**
 * Constructor initializes parsing state
 */
HttpRequest::HttpRequest(void) : _state(REQUEST_LINE), _contentLength(0), 
	_chunkSize(0), _chunked(false), _connectionError(false)
{
}

/**
 * Copy constructor
 */
HttpRequest::HttpRequest(const HttpRequest& other) : 
	_method(other._method),
	_uri(other._uri),
	_httpVersion(other._httpVersion),
	_path(other._path),
	_query(other._query),
	_headers(other._headers),
	_body(other._body),
	_state(other._state),
	_buffer(other._buffer),
	_contentLength(other._contentLength),
	_chunkSize(other._chunkSize),
	_chunked(other._chunked),
	_connectionError(other._connectionError)
{
}

/**
 * Destructor
 */
HttpRequest::~HttpRequest(void)
{
}

/**
 * Assignment operator
 */
HttpRequest&	HttpRequest::operator=(const HttpRequest& other)
{
	if (this != &other)
	{
		_method = other._method;
		_uri = other._uri;
		_httpVersion = other._httpVersion;
		_path = other._path;
		_query = other._query;
		_headers = other._headers;
		_body = other._body;
		_state = other._state;
		_buffer = other._buffer;
		_contentLength = other._contentLength;
		_chunkSize = other._chunkSize;
		_chunked = other._chunked;
		_connectionError = other._connectionError;
	}
	return *this;
}

/**
 * Read and parse data from the client socket
 * Returns true when the request is complete
 */
bool	HttpRequest::read(Socket& clientSocket)
{
	const size_t BUFFER_SIZE = 4096;
	char buffer[BUFFER_SIZE];
	
	ssize_t bytesRead = clientSocket.recv(buffer, BUFFER_SIZE);
	
	_logger.tempOss << "HttpRequest::read() received " << bytesRead 
	    << " bytes";
		_logger.debug();
	
	if (bytesRead <= 0)
	{
	    if (bytesRead == 0)
		{
			// In non-blocking I/O, recv() returning 0 can mean either:
			// 1. Client actually closed connection 
			// 2. No data available right now (EAGAIN/EWOULDBLOCK)
			// 
			// If we haven't started parsing yet, or if we're complete/error, 
			// then 0 bytes likely means connection closed.
			// But if we're in the middle of reading body data, it might just
			// mean no more data available right now.
			if (_state == REQUEST_LINE || _state == COMPLETE || _state == ERROR)
			{
				_logger.tempOss << "Client closed connection";
				_logger.debug();
				_connectionError = true;
			}
			else
			{
				// We're in the middle of parsing, this might just be no data available
				_logger.tempOss << "No more data available, request incomplete";
				_logger.debug();
				// Don't set _connectionError - let server keep connection for more data
			}
		}
	    else
		{
			_logger.tempOss << "Error reading from socket: " << strerror(errno);
			_logger.error();
			_connectionError = true;
		}
		return false;
	}
		
	// Append the received data to our buffer
	_buffer.append(buffer, bytesRead);
	
	// Print first line of the buffer for debugging
	size_t firstLineEnd = _buffer.find("\r\n");
	if (firstLineEnd != std::string::npos){
	    _logger.tempOss << "First line of request: " << _buffer.substr(0, firstLineEnd);
		_logger.debug();
	}
	else
	{
		_logger.tempOss << "Request buffer (incomplete): " << _buffer;
		_logger.debug();
	}
	
	// Process the buffer based on current state
	bool done = false;
	
	while (!done)
	{
	    _logger.tempOss << "Request parse state: " << _state;
		_logger.debug();
		switch (_state)
		{
			case REQUEST_LINE:
				done = !parseRequestLine();
				break;
			case HEADERS:
				done = !parseHeaders();
				break;
			case BODY:
				done = !parseBody();
				break;
			case CHUNKED_SIZE:
				done = !parseChunkedSize();
				break;
			case CHUNKED_DATA:
				done = !parseChunkedData();
				break;
			case CHUNKED_END:
				// Check for the final CRLF
				if (_buffer.size() >= 2 && _buffer.substr(0, 2) == "\r\n")
				{
					_buffer = _buffer.substr(2);
					_state = COMPLETE;
					_logger.tempOss << "Request is COMPLETE after chunked end";
					_logger.debug();
				}
				else
				{
					done = true;
				}
				break;
			case COMPLETE:
			    _logger.tempOss << "Request is COMPLETE";
				_logger.debug();
				return true;
			case ERROR:
			    _logger.tempOss << "Request has ERROR state";
				_logger.debug();
				return true;
			default:
				done = true;
		}
	}
	
	bool isComplete = (_state == COMPLETE || _state == ERROR);
	_logger.tempOss << "Returning from read(), request is " 
	    << (isComplete ? "COMPLETE" : "INCOMPLETE");
		_logger.debug();
	
	return isComplete;
}

/**
 * Parse the request line (METHOD URI HTTP/VERSION)
 */
bool	HttpRequest::parseRequestLine(void)
{
	size_t endPos = _buffer.find("\r\n");
	
	if (endPos == std::string::npos)
	{
	    _logger.tempOss << "Request line incomplete, waiting for more data";
		_logger.debug();
		return false;
	}
		
	std::string line = _buffer.substr(0, endPos);
	_buffer = _buffer.substr(endPos + 2);
	
	_logger.tempOss << "Parsing request line: " << line;
	_logger.debug();
	
	std::istringstream lineStream(line);
	if (!(lineStream >> _method >> _uri >> _httpVersion))
	{
	    _logger.tempOss << "Failed to parse request line";
		_logger.debug();
		_state = ERROR;
		return false;
	}
	
	_logger.tempOss << "Method=" << _method << ", URI=" << _uri 
	    << ", Version=" << _httpVersion;
		_logger.debug();
	
	// Parse the URI into path and query
	size_t queryPos = _uri.find('?');
	if (queryPos != std::string::npos)
	{
		_path = _uri.substr(0, queryPos);
		_query = _uri.substr(queryPos + 1);
	}
	else
	{
		_path = _uri;
	}
	
	_logger.tempOss << "Path=" << _path << ", Query=" << _query;
	_logger.debug();
	
	_state = HEADERS;
	return true;
}

/**
 * Parse HTTP headers
 */
bool	HttpRequest::parseHeaders(void)
{
	while (true)
	{
		size_t endPos = _buffer.find("\r\n");
		
		if (endPos == std::string::npos)
		{
		    _logger.tempOss << "Headers incomplete, waiting for more data";
			_logger.debug();
			return false;
		}
			
		// If we find an empty line, headers are complete
		if (endPos == 0)
		{
			_buffer = _buffer.substr(2);
			
			_logger.tempOss << "End of headers found";
			_logger.debug();
			
			// Determine the next state based on headers
			std::string transferEncoding = getHeader("Transfer-Encoding");
			std::transform(transferEncoding.begin(), transferEncoding.end(), 
				transferEncoding.begin(), ::tolower);
				
			if (transferEncoding.find("chunked") != std::string::npos)
			{
			    _logger.tempOss << "Found chunked encoding";
				_logger.debug();
				_chunked = true;
				_state = CHUNKED_SIZE;
			}
			else
			{
				std::string contentLengthStr = getHeader("Content-Length");
				if (!contentLengthStr.empty())
				{
					_contentLength = strtoul(contentLengthStr.c_str(), NULL, 10);
					_logger.tempOss << "Content-Length: " << _contentLength;
					_logger.debug();
					_state = BODY;
				}
				else
				{
					// No content expected, request is complete
					_logger.tempOss << "No body expected, request is complete";
					_logger.debug();
					_state = COMPLETE;
				}
			}
			
			return true;
		}
		
		std::string line = _buffer.substr(0, endPos);
		_buffer = _buffer.substr(endPos + 2);
		
		_logger.tempOss << "Parsing header line: " << line;
		_logger.debug();
		
		size_t colonPos = line.find(':');
		if (colonPos == std::string::npos)
		{
		    _logger.tempOss << "Invalid header line: " << line;
			_logger.debug();
			_state = ERROR;
			return false;
		}
		
		std::string name = line.substr(0, colonPos);
		std::string value = line.substr(colonPos + 1);
		
		// Trim leading and trailing whitespace from value
		size_t valueStart = value.find_first_not_of(" \t");
		if (valueStart != std::string::npos)
		{
			size_t valueEnd = value.find_last_not_of(" \t");
			value = value.substr(valueStart, valueEnd - valueStart + 1);
		}
		else
		{
			value = "";
		}
		
		_logger.tempOss << "Header: [" << name << "] = [" << value << "]";
		_logger.debug();
		_headers[name] = value;
	}
}

/**
 * Parse request body based on Content-Length
 */
bool	HttpRequest::parseBody(void)
{
    _logger.tempOss << "Parsing body, have " << _buffer.size() 
        << " bytes, need " << _contentLength;
		_logger.debug();
        
	if (_buffer.size() >= _contentLength)
	{
		_body.append(_buffer.substr(0, _contentLength));
		_buffer = _buffer.substr(_contentLength);
		_state = COMPLETE;
		_logger.tempOss << "Body complete with " << _body.size() << " bytes";
		_logger.debug();
		return true;
	}
	
	_logger.tempOss << "Body incomplete, waiting for more data";
	_logger.debug();
	return false;
}

/**
 * Parse chunked encoding size line
 */
bool	HttpRequest::parseChunkedSize(void)
{
	size_t endPos = _buffer.find("\r\n");
	
	if (endPos == std::string::npos)
	{
	    _logger.tempOss << "Chunk size line incomplete";
		_logger.debug();
		return false;
	}
		
	std::string line = _buffer.substr(0, endPos);
	_buffer = _buffer.substr(endPos + 2);
	
	// Parse the chunk size (hex)
	char* endPtr;
	_chunkSize = strtoul(line.c_str(), &endPtr, 16);
	
	_logger.tempOss << "Chunk size: " << _chunkSize << " bytes";
	_logger.debug();
	
	if (_chunkSize == 0)
	{
		// Final chunk, look for trailing headers (not implemented)
		_logger.tempOss << "Final chunk (size 0) received";
		_logger.debug();
		_state = CHUNKED_END;
	}
	else
	{
		_state = CHUNKED_DATA;
	}
	
	return true;
}

/**
 * Parse chunked encoding data
 */
bool	HttpRequest::parseChunkedData(void)
{
    _logger.tempOss << "Parsing chunk data, have " << _buffer.size() 
        << " bytes, need " << _chunkSize + 2;
		_logger.debug();
        
	if (_buffer.size() >= _chunkSize + 2)  // +2 for CRLF
	{
		_body.append(_buffer.substr(0, _chunkSize));
		_buffer = _buffer.substr(_chunkSize + 2);  // Skip CRLF
		_state = CHUNKED_SIZE;
		
		_logger.tempOss << "Chunk data complete, body now " 
		    << _body.size() << " bytes";
		_logger.debug();
		return true;
	}
	
	_logger.tempOss << "Chunk data incomplete, waiting for more data";
	_logger.debug();
	return false;
}

/**
 * Process the request and generate a response
 */
HttpResponse	HttpRequest::process(const Config& config)
{
    _logger.tempOss << "Processing request: " << _method << " " 
        << _uri << " " << _httpVersion;
		_logger.debug();
    
	HttpResponse response;
	
	// Extract host and port from Host header
	std::string host = getHeader("Host");
	int port = 80;  // Default
	
	_logger.tempOss << "Host header: " << host;
	_logger.debug();
	
	size_t colonPos = host.find(':');
	if (colonPos != std::string::npos)
	{
		port = atoi(host.substr(colonPos + 1).c_str());
		host = host.substr(0, colonPos);
		_logger.tempOss << "Extracted host=" << host << ", port=" << port;
		_logger.debug();
	}
	
	// Find the appropriate server configuration
	const ServerConfig* server = config.findServer(host, port, host);
	
	if (!server)
	{
	    _logger.tempOss << "No matching server configuration found";
		_logger.debug();
		response.setStatus(404);
		response.setBody(config.getDefaultErrorPage(404));
		return response;
	}
	
	_logger.tempOss << "Found matching server for " << host 
	    << ":" << port;
		_logger.debug();
	
	// Find the appropriate location configuration
	const LocationConfig* location = findLocation(*server);
	
	if (!location)
	{
	    _logger.tempOss << "No matching location configuration found for " 
	        << _path;
		_logger.debug();
		response.setStatus(404);
		response.setBody(config.getDefaultErrorPage(404));
		return response;
	}
	
	_logger.tempOss << "Found matching location: " << location->path;
	_logger.debug();
	
	// Check if method is allowed
	if (!location->allowedMethods.empty() && 
		location->allowedMethods.find(_method) == location->allowedMethods.end())
	{
	    _logger.tempOss << "Method " << _method << " not allowed";
		_logger.debug();
		response.setStatus(405);
		response.setBody(config.getDefaultErrorPage(405));
		return response;
	}
	
	// Handle redirections
	if (!location->redirect.empty())
	{
	    _logger.tempOss << "Redirecting to " << location->redirect;
		_logger.debug();
		response.setStatus(301);
		response.addHeader("Location", location->redirect);
		return response;
	}
	
	// Handle different HTTP methods
	if (_method == "GET")
	{
		return handleGet(*location, response, config);
	}
	else if (_method == "POST")
	{
		return handlePost(*location, response, config);
	}
	else if (_method == "DELETE")
	{
		return handleDelete(*location, response, config);
	}
	else
	{
		_logger.tempOss << "Unsupported method " << _method;
		_logger.debug();
		response.setStatus(501);
		response.setBody(config.getDefaultErrorPage(501));
		return response;
	}
}

/**
 * Find the location configuration for this request
 */
const LocationConfig*	HttpRequest::findLocation(const ServerConfig& server) const
{
	// Find the best matching location
	const LocationConfig* bestMatch = NULL;
	size_t bestMatchLength = 0;
	
	for (size_t i = 0; i < server.locations.size(); i++)
	{
		const LocationConfig& location = server.locations[i];
		
		// Check if the location path is a prefix of the request path
		if (_path.find(location.path) == 0)
		{
			size_t matchLength = location.path.length();
			
			// If this is a better (longer) match, use it
			if (matchLength > bestMatchLength)
			{
				bestMatch = &location;
				bestMatchLength = matchLength;
			}
		}
	}
	
	return bestMatch;
}

/**
 * Get the request method
 */
const std::string&	HttpRequest::getMethod(void) const
{
	return _method;
}

/**
 * Get the request URI
 */
const std::string&	HttpRequest::getUri(void) const
{
	return _uri;
}

/**
 * Get the request HTTP version
 */
const std::string&	HttpRequest::getHttpVersion(void) const
{
	return _httpVersion;
}

/**
 * Get a specific header value
 */
std::string	HttpRequest::getHeader(const std::string& name) const
{
	std::map<std::string, std::string>::const_iterator it = _headers.find(name);
	if (it != _headers.end())
		return it->second;
	return "";
}

/**
 * Get all headers
 */
const std::map<std::string, std::string>&	HttpRequest::getHeaders(void) const
{
	return _headers;
}

/**
 * Get the request body
 */
const std::string&	HttpRequest::getBody(void) const
{
	return _body;
}

/**
 * Handle GET request for static file serving
 */
HttpResponse	HttpRequest::handleGet(const LocationConfig& location, 
	HttpResponse& response, const Config& config)
{
	_logger.tempOss << "Handling GET request for " << _path;
	_logger.debug();
	// Build the full file path
	std::string fullPath = location.root + _path;
	
	// Check for path traversal attacks
	if (!isPathSafe(_path, location.root))
	{
		_logger.tempOss << "Path traversal detected: " << _path;
		_logger.debug();
		response.setStatus(403);
		response.setBody(config.getDefaultErrorPage(403));
		return response;
	}
	
	// Check if this is a CGI request
	if (CgiHandler::isCgiFile(fullPath, location))
	{
		_logger.tempOss << "Detected CGI file, delegating to CGI handler";
		_logger.debug();
		return handleCgi(location, response, config);
	}
	
	// Check if the path is a directory
	struct stat pathStat;
	bool isDirectory = (stat(fullPath.c_str(), &pathStat) == 0 && S_ISDIR(pathStat.st_mode));
	
	if (isDirectory)
	{
		// If path doesn't end with '/', redirect to add trailing slash
		if (!_path.empty() && _path[_path.length() - 1] != '/')
		{
			response.setStatus(301);
			response.addHeader("Location", _path + "/");
			return response;
		}
		
		// Path ends with '/', try to serve index file first
		std::string indexPath = fullPath;
		if (!location.index.empty())
		{
			indexPath += location.index;
		}
		else
		{
			indexPath += "index.html";
		}
		
		// Check if index file exists
		std::ifstream indexFile(indexPath.c_str(), std::ios::binary);
		if (indexFile.is_open())
		{
			// Serve the index file
			std::string content((std::istreambuf_iterator<char>(indexFile)), 
			                   std::istreambuf_iterator<char>());
			indexFile.close();
			
			_logger.tempOss << "Successfully read " << content.size() 
			    << " bytes from index file " << indexPath;
			_logger.debug();
			
			response.setStatus(200);
			response.setBody(content);
			response.addHeader("Content-Type", getMimeType(indexPath));
			return response;
		}
		
		// No index file found, check if autoindex is enabled
		if (location.autoindex)
		{
			return generateDirectoryListing(location, response);
		}
		else
		{
			// Directory exists but no index file and autoindex disabled
			response.setStatus(403);
			response.setBody(config.getDefaultErrorPage(403));
			return response;
		}
	}
	
	// Not a directory, handle as regular file
	// If path ends with '/', try to serve index file (shouldn't happen after directory check)
	if (!_path.empty() && _path[_path.length() - 1] == '/')
	{
		if (!location.index.empty())
		{
			fullPath += location.index;
		}
		else
		{
			fullPath += "index.html";
		}
	}
	
	// Try to open and read the file
	std::ifstream file(fullPath.c_str(), std::ios::binary);
	
	if (!file.is_open())
	{
		_logger.tempOss << "File not found: " << fullPath;
		_logger.debug();
		
		response.setStatus(404);
		response.setBody(config.getDefaultErrorPage(404));
		return response;
	}
	
	// Read file content
	std::string content((std::istreambuf_iterator<char>(file)), 
	                   std::istreambuf_iterator<char>());
	file.close();
	
	_logger.tempOss << "Successfully read " << content.size() 
	    << " bytes from " << fullPath;
		_logger.debug();
	
	// Set response
	response.setStatus(200);
	response.setBody(content);
	response.addHeader("Content-Type", getMimeType(fullPath));
	
	return response;
}

HttpResponse HttpRequest::handlePost(LocationConfig const &location, 
	HttpResponse& response, Config const &config) {
	_logger.tempOss << "Handling POST request for " << _path;
	_logger.debug();
	
	// Build the full file path to check for CGI
	std::string fullPath = location.root + _path;
	
	// Check if this is a CGI request
	if (CgiHandler::isCgiFile(fullPath, location)) {
		_logger.tempOss << "Detected CGI file, delegating to CGI handler";
		_logger.debug();
		return handleCgi(location, response, config);
	}
	
	// Check if upload is allowed for this location
	if (location.uploadStore.empty()) {
		_logger.tempOss << "Upload not allowed for this location";
		_logger.debug();
		response.setStatus(403);
		response.setBody(config.getDefaultErrorPage(403));
		return response;
	}
	
	// Parse Content-Type header to handle multipart/form-data
	std::string contentType = getHeader("Content-Type");
	
	if (contentType.find("multipart/form-data") != std::string::npos) {
		return handleFileUpload(location, response, config);
	}
	else if (contentType.find("application/x-www-form-urlencoded") !=
	std::string::npos) {
		return handleFormData(response);
	}
	else {
		// Simple file upload - treat body as raw file content
		// Ensure upload directory exists
		struct stat dirStat;
		if (stat(location.uploadStore.c_str(), &dirStat) != 0 || !S_ISDIR(dirStat.st_mode))
		{
			_logger.tempOss << "Upload directory does not exist: " << location.uploadStore;
			_logger.debug();
			response.setStatus(500);
			response.setBody(config.getDefaultErrorPage(500));
			return response;
		}
		
		std::ostringstream oss;
		oss << "upload_" << time(NULL);
		std::string filename = oss.str();
		std::string uploadPath = location.uploadStore + "/" + filename;
		
		std::ofstream file(uploadPath.c_str(), std::ios::binary);
		if (!file.is_open()) {
			_logger.tempOss << "Failed to create upload file: " << 
			uploadPath;
			_logger.debug();
			response.setStatus(500);
			response.setBody(config.getDefaultErrorPage(500));
			return response;
		}
		
		file.write(_body.c_str(), _body.size());
		file.close();
		
		_logger.tempOss << "Successfully uploaded " << _body.size() 
		    << " bytes to " << uploadPath;
			_logger.debug();
		
		response.setStatus(303);
		response.addHeader("Location", "/sucessupload.html");
		return response;
	}
	
	return response;
}

/**
 * Handle DELETE request for file deletion
 */
HttpResponse	HttpRequest::handleDelete(const LocationConfig& location, 
	HttpResponse& response, const Config& config)
{
	_logger.tempOss << "Handling DELETE request for " << _path;
	_logger.debug();
	
	// Build the full file path
	std::string fullPath = location.root + _path;
	
	// Check for path traversal attacks
	if (!isPathSafe(_path, location.root))
	{
		_logger.tempOss << "Path traversal detected: " << _path;
		_logger.debug();
		response.setStatus(403);
		response.setBody(config.getDefaultErrorPage(403));
		return response;
	}
	
	// Check if file exists
	std::ifstream file(fullPath.c_str());
	if (!file.good())
	{
		_logger.tempOss << "File not found for deletion: " << fullPath;
		_logger.debug();
		response.setStatus(404);
		response.setBody(config.getDefaultErrorPage(404));
		return response;
	}
	file.close();
	
	// Try to delete the file
	if (std::remove(fullPath.c_str()) != 0)
	{
		_logger.tempOss << "Failed to delete file: " << fullPath;
		_logger.debug();
		response.setStatus(500);
		response.setBody(config.getDefaultErrorPage(500));
		return response;
	}
	
	_logger.tempOss << "Successfully deleted file: " << fullPath;
	_logger.debug();
	
	response.setStatus(204); // No Content
	return response;
}

/**
 * Get MIME type for file extension
 */
std::string	HttpRequest::getMimeType(const std::string& filename) const
{
	size_t dotPos = filename.find_last_of('.');
	if (dotPos == std::string::npos)
		return "application/octet-stream";
	
	std::string ext = filename.substr(dotPos + 1);
	
	// Convert to lowercase for comparison
	for (size_t i = 0; i < ext.length(); ++i)
		ext[i] = std::tolower(ext[i]);
	
	if (ext == "html" || ext == "htm")
		return "text/html";
	else if (ext == "css")
		return "text/css";
	else if (ext == "js")
		return "application/javascript";
	else if (ext == "json")
		return "application/json";
	else if (ext == "xml")
		return "application/xml";
	else if (ext == "txt")
		return "text/plain";
	else if (ext == "jpg" || ext == "jpeg")
		return "image/jpeg";
	else if (ext == "png")
		return "image/png";
	else if (ext == "gif")
		return "image/gif";
	else if (ext == "svg")
		return "image/svg+xml";
	else if (ext == "ico")
		return "image/x-icon";
	else if (ext == "pdf")
		return "application/pdf";
	else if (ext == "zip")
		return "application/zip";
	else if (ext == "mp4")
		return "video/mp4";
	else if (ext == "mp3")
		return "audio/mpeg";
	else
		return "application/octet-stream";
}

/**
 * Check if path is safe and within allowed directory
 */
bool	HttpRequest::isPathSafe(const std::string& path, const std::string& root) const
{
	(void)root; // Unused parameter
	
	// Check for path traversal patterns
	if (path.find("..") != std::string::npos)
		return false;
	
	// Check for absolute paths trying to escape root
	if (path.find("/..") != std::string::npos)
		return false;
	
	// Check for null bytes
	if (path.find('\0') != std::string::npos)
		return false;
	
	return true;
}

/**
 * Generate directory listing for autoindex
 */
HttpResponse	HttpRequest::generateDirectoryListing(const LocationConfig& location, 
	HttpResponse& response)
{
	_logger.tempOss << "Generating directory listing for " << _path;
	_logger.debug();
	
	std::string dirPath = location.root + _path;
	DIR* dir = opendir(dirPath.c_str());
	
	if (!dir)
	{
		_logger.tempOss << "Failed to open directory: " << dirPath;
		_logger.debug();
		response.setStatus(403);
		return response;
	}
	
	std::ostringstream html;
	html << "<!DOCTYPE html>\n<html>\n<head>\n";
	html << "<title>Index of " << _path << "</title>\n";
	html << "<style>body{font-family:Arial,sans-serif;margin:40px;}</style>\n";
	html << "</head>\n<body>\n";
	html << "<h1>Index of " << _path << "</h1>\n<hr>\n<pre>\n";
	
	if (_path != "/")
	{
		html << "<a href=\"../\">../</a>\n";
	}
	
	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL)
	{
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		
		std::string fullItemPath = dirPath + "/" + entry->d_name;
		struct stat statBuf;
		
		if (stat(fullItemPath.c_str(), &statBuf) == 0)
		{
			html << "<a href=\"" << entry->d_name;
			if (S_ISDIR(statBuf.st_mode))
				html << "/";
			html << "\">" << entry->d_name;
			if (S_ISDIR(statBuf.st_mode))
				html << "/";
			html << "</a>\n";
		}
	}
	
	html << "</pre>\n<hr>\n</body>\n</html>";
	closedir(dir);
	
	response.setStatus(200);
	response.setBody(html.str());
	response.addHeader("Content-Type", "text/html");
	
	return response;
}

/**
 * Handle multipart/form-data file upload
 */
HttpResponse	HttpRequest::handleFileUpload(const LocationConfig& location, 
	HttpResponse& response, const Config& config)
{
	_logger.tempOss << "Handling multipart file upload";
	_logger.debug();
	
	// Parse boundary from Content-Type header
	std::string contentType = getHeader("Content-Type");
	size_t boundaryPos = contentType.find("boundary=");
	
	if (boundaryPos == std::string::npos)
	{
		_logger.tempOss << "No boundary found in multipart data";
		_logger.debug();
		response.setStatus(400);
		response.setBody(config.getDefaultErrorPage(400));
		return response;
	}
	
	// Extract boundary, handling quotes if present
	std::string boundaryValue = contentType.substr(boundaryPos + 9);
	if (!boundaryValue.empty() && boundaryValue[0] == '"')
	{
		size_t endQuote = boundaryValue.find('"', 1);
		if (endQuote != std::string::npos)
			boundaryValue = boundaryValue.substr(1, endQuote - 1);
		else
			boundaryValue = boundaryValue.substr(1);
	}
	else
	{
		// Remove any trailing semicolon or other parameters
		size_t semicolon = boundaryValue.find(';');
		if (semicolon != std::string::npos)
			boundaryValue = boundaryValue.substr(0, semicolon);
	}
	
	std::string boundary = "--" + boundaryValue;
	_logger.tempOss << "Using boundary: " << boundary;
	_logger.debug();
	
	// Simple multipart parsing - find file content between boundaries
	size_t startPos = _body.find(boundary);
	if (startPos == std::string::npos)
	{
		response.setStatus(400);
		response.setBody(config.getDefaultErrorPage(400));
		return response;
	}
	
	// Find filename in headers - more robust parsing
	std::string filename = "uploaded_file";
	size_t filenamePos = _body.find("filename=", startPos);
	
	if (filenamePos != std::string::npos)
	{
		size_t filenameStart = filenamePos + 9;
		char delimiter = '"';
		
		// Check if filename is quoted
		if (filenameStart < _body.length() && _body[filenameStart] == '"')
		{
			filenameStart++;
			delimiter = '"';
		}
		else
		{
			// Unquoted filename, delimiter is space, semicolon or CRLF
			delimiter = ' ';
		}
		
		size_t filenameEnd = _body.find(delimiter, filenameStart);
		if (delimiter == ' ')
		{
			// For unquoted, also check for semicolon and CRLF
			size_t semicolonPos = _body.find(';', filenameStart);
			size_t crlfPos = _body.find("\r\n", filenameStart);
			
			filenameEnd = filenameEnd < semicolonPos ? filenameEnd : semicolonPos;
			filenameEnd = filenameEnd < crlfPos ? filenameEnd : crlfPos;
		}
		
		if (filenameEnd != std::string::npos && filenameEnd > filenameStart)
		{
			filename = _body.substr(filenameStart, filenameEnd - filenameStart);
			// Remove any path information for security
			size_t lastSlash = filename.find_last_of("/\\");
			if (lastSlash != std::string::npos)
				filename = filename.substr(lastSlash + 1);
		}
	}
	
	// Ensure filename is safe
	if (filename.empty() || filename == "." || filename == "..")
		filename = "uploaded_file";
	
	// Find file content (after double CRLF)
	size_t contentStart = _body.find("\r\n\r\n", startPos);
	if (contentStart == std::string::npos)
	{
		response.setStatus(400);
		response.setBody(config.getDefaultErrorPage(400));
		return response;
	}
	contentStart += 4;
	
	// Find end boundary
	size_t contentEnd = _body.find("\r\n--", contentStart);
	if (contentEnd == std::string::npos)
		contentEnd = _body.length();
	
	std::string fileContent = _body.substr(contentStart, contentEnd - contentStart);
	
	// Ensure upload directory exists
	struct stat dirStat;
	if (stat(location.uploadStore.c_str(), &dirStat) != 0 || !S_ISDIR(dirStat.st_mode))
	{
		_logger.tempOss << "Upload directory does not exist: " << location.uploadStore;
		_logger.debug();
		response.setStatus(500);
		response.setBody(config.getDefaultErrorPage(500));
		return response;
	}
	
	// Save file
	std::string uploadPath = location.uploadStore + "/" + filename;
	std::ofstream file(uploadPath.c_str(), std::ios::binary);
	
	if (!file.is_open())
	{
		_logger.tempOss << "Failed to create upload file: " << uploadPath ;
		_logger.debug();
		response.setStatus(500);
		response.setBody(config.getDefaultErrorPage(500));
		return response;
	}
	
	file.write(fileContent.c_str(), fileContent.size());
	file.close();

	if (fileContent.size() > config.getServers()[0].clientMaxBodySize)
	{
		_logger.tempOss << "File too big: size (" << fileContent.size()  << ")";
		_logger.error();
		response.setStatus(500);
		response.setBody(config.getDefaultErrorPage(500));
		return response;
	}
	
	_logger.tempOss << "Successfully uploaded " << fileContent.size() 
	    << " bytes to " << uploadPath;
		_logger.debug();
	
	response.setStatus(303);
	response.addHeader("Location", "/sucessupload.html");
	
	return response;
}

HttpResponse HttpRequest::handleFormData(HttpResponse &response) {
	_logger.tempOss << "Handling form data";
	_logger.debug();
	
	response.setStatus(200);
	response.setBody(
		"<!DOCTYPE html>"
    "<html>"
    "<head>"
        "<link rel=\"stylesheet\" href=\"style.css\" />"
    "</head>"
    "<body>"
        "<h1>Form Data Received!</h1>"
        "<div class=\"success\">"
            "<img src=\"images/suggestion-box.gif\" alt=\"printing form\" class=\"data-image\" />"
						"<h2>"+ _body +"</h2>"
        "</div>"
        "<p>Go back <a href=\"index.html\">Home</a></p>"
    "</body>"
    "</html>"
	);
	response.addHeader("Content-Type", "text/html");
	
	return response;
}

/**
 * Handle CGI request execution
 */
HttpResponse	HttpRequest::handleCgi(const LocationConfig& location, 
	HttpResponse& response, const Config& config)
{
	(void)config; // May be used for additional configuration
	
	_logger.tempOss << "Handling CGI request for " << _path;
	_logger.debug();
	
	// Build the full script path
	std::string scriptPath = location.root + _path;
	
	// Check for path traversal attacks
	if (!isPathSafe(_path, location.root))
	{
		_logger.tempOss << "Path traversal detected: " << _path;
		_logger.debug();
		response.setStatus(403);
		response.setBody("Forbidden: Path traversal detected");
		return response;
	}
	
	// Check if it's a CGI file
	if (!CgiHandler::isCgiFile(scriptPath, location))
	{
		_logger.tempOss << "File is not a CGI script: " << scriptPath;
		_logger.debug();
		response.setStatus(403);
		response.setBody("Forbidden: Not a CGI script");
		return response;
	}
	
	// Use CgiHandler to execute the script
	CgiHandler cgiHandler;
	return cgiHandler.handleCgiRequest(*this, location, scriptPath);
}

/**
 * Check if there was a connection error (should close connection)
 */
bool	HttpRequest::hasConnectionError(void) const
{
	return _connectionError;
}
