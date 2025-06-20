/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: josfelip <josfelip@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/26 13:50:42 by josfelip          #+#    #+#             */
/*   Updated: 2025/04/02 20:42:57 by josfelip         ###   ########.fr       */
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
#include <sys/types.h>

/**
 * Constructor initializes parsing state
 */
HttpRequest::HttpRequest(void) : _state(REQUEST_LINE), _contentLength(0), 
	_chunkSize(0), _chunked(false)
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
	_chunked(other._chunked)
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
	
	std::cout << "DEBUG: HttpRequest::read() received " << bytesRead 
	    << " bytes" << std::endl;
	
	if (bytesRead <= 0)
	{
	    if (bytesRead == 0)
	        std::cout << "DEBUG: Client closed connection" << std::endl;
	    else
		{
			std::cout << "DEBUG: Error reading from socket: "  
			<< strerror(errno) << std::endl;			
		}
		return false;
	}
		
	// Append the received data to our buffer
	_buffer.append(buffer, bytesRead);
	
	// Print first line of the buffer for debugging
	size_t firstLineEnd = _buffer.find("\r\n");
	if (firstLineEnd != std::string::npos)
	    std::cout << "DEBUG: First line of request: " 
	        << _buffer.substr(0, firstLineEnd) << std::endl;
	else
	    std::cout << "DEBUG: Request buffer (incomplete): " 
	        << _buffer << std::endl;
	
	// Process the buffer based on current state
	bool done = false;
	
	while (!done)
	{
	    std::cout << "DEBUG: Request parse state: " << _state << std::endl;
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
					std::cout << "DEBUG: Request is COMPLETE after chunked end" << std::endl;
				}
				else
				{
					done = true;
				}
				break;
			case COMPLETE:
			    std::cout << "DEBUG: Request is COMPLETE" << std::endl;
				return true;
			case ERROR:
			    std::cout << "DEBUG: Request has ERROR state" << std::endl;
				return true;
			default:
				done = true;
		}
	}
	
	bool isComplete = (_state == COMPLETE || _state == ERROR);
	std::cout << "DEBUG: Returning from read(), request is " 
	    << (isComplete ? "COMPLETE" : "INCOMPLETE") << std::endl;
	
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
	    std::cout << "DEBUG: Request line incomplete, waiting for more data" << std::endl;
		return false;
	}
		
	std::string line = _buffer.substr(0, endPos);
	_buffer = _buffer.substr(endPos + 2);
	
	std::cout << "DEBUG: Parsing request line: " << line << std::endl;
	
	std::istringstream lineStream(line);
	if (!(lineStream >> _method >> _uri >> _httpVersion))
	{
	    std::cout << "DEBUG: Failed to parse request line" << std::endl;
		_state = ERROR;
		return false;
	}
	
	std::cout << "DEBUG: Method=" << _method << ", URI=" << _uri 
	    << ", Version=" << _httpVersion << std::endl;
	
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
	
	std::cout << "DEBUG: Path=" << _path << ", Query=" << _query << std::endl;
	
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
		    std::cout << "DEBUG: Headers incomplete, waiting for more data" << std::endl;
			return false;
		}
			
		// If we find an empty line, headers are complete
		if (endPos == 0)
		{
			_buffer = _buffer.substr(2);
			
			std::cout << "DEBUG: End of headers found" << std::endl;
			
			// Determine the next state based on headers
			std::string transferEncoding = getHeader("Transfer-Encoding");
			std::transform(transferEncoding.begin(), transferEncoding.end(), 
				transferEncoding.begin(), ::tolower);
				
			if (transferEncoding.find("chunked") != std::string::npos)
			{
			    std::cout << "DEBUG: Found chunked encoding" << std::endl;
				_chunked = true;
				_state = CHUNKED_SIZE;
			}
			else
			{
				std::string contentLengthStr = getHeader("Content-Length");
				if (!contentLengthStr.empty())
				{
					_contentLength = strtoul(contentLengthStr.c_str(), NULL, 10);
					std::cout << "DEBUG: Content-Length: " << _contentLength << std::endl;
					_state = BODY;
				}
				else
				{
					// No content expected, request is complete
					std::cout << "DEBUG: No body expected, request is complete" << std::endl;
					_state = COMPLETE;
				}
			}
			
			return true;
		}
		
		std::string line = _buffer.substr(0, endPos);
		_buffer = _buffer.substr(endPos + 2);
		
		std::cout << "DEBUG: Parsing header line: " << line << std::endl;
		
		size_t colonPos = line.find(':');
		if (colonPos == std::string::npos)
		{
		    std::cout << "DEBUG: Invalid header line: " << line << std::endl;
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
		
		std::cout << "DEBUG: Header: [" << name << "] = [" << value << "]" << std::endl;
		_headers[name] = value;
	}
}

/**
 * Parse request body based on Content-Length
 */
bool	HttpRequest::parseBody(void)
{
    std::cout << "DEBUG: Parsing body, have " << _buffer.size() 
        << " bytes, need " << _contentLength << std::endl;
        
	if (_buffer.size() >= _contentLength)
	{
		_body.append(_buffer.substr(0, _contentLength));
		_buffer = _buffer.substr(_contentLength);
		_state = COMPLETE;
		std::cout << "DEBUG: Body complete with " << _body.size() << " bytes" << std::endl;
		return true;
	}
	
	std::cout << "DEBUG: Body incomplete, waiting for more data" << std::endl;
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
	    std::cout << "DEBUG: Chunk size line incomplete" << std::endl;
		return false;
	}
		
	std::string line = _buffer.substr(0, endPos);
	_buffer = _buffer.substr(endPos + 2);
	
	// Parse the chunk size (hex)
	char* endPtr;
	_chunkSize = strtoul(line.c_str(), &endPtr, 16);
	
	std::cout << "DEBUG: Chunk size: " << _chunkSize << " bytes" << std::endl;
	
	if (_chunkSize == 0)
	{
		// Final chunk, look for trailing headers (not implemented)
		std::cout << "DEBUG: Final chunk (size 0) received" << std::endl;
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
    std::cout << "DEBUG: Parsing chunk data, have " << _buffer.size() 
        << " bytes, need " << _chunkSize + 2 << std::endl;
        
	if (_buffer.size() >= _chunkSize + 2)  // +2 for CRLF
	{
		_body.append(_buffer.substr(0, _chunkSize));
		_buffer = _buffer.substr(_chunkSize + 2);  // Skip CRLF
		_state = CHUNKED_SIZE;
		
		std::cout << "DEBUG: Chunk data complete, body now " 
		    << _body.size() << " bytes" << std::endl;
		return true;
	}
	
	std::cout << "DEBUG: Chunk data incomplete, waiting for more data" << std::endl;
	return false;
}

/**
 * Process the request and generate a response
 */
HttpResponse	HttpRequest::process(const Config& config)
{
    std::cout << "DEBUG: Processing request: " << _method << " " 
        << _uri << " " << _httpVersion << std::endl;
    
	HttpResponse response;
	
	// Extract host and port from Host header
	std::string host = getHeader("Host");
	int port = 80;  // Default
	
	std::cout << "DEBUG: Host header: " << host << std::endl;
	
	size_t colonPos = host.find(':');
	if (colonPos != std::string::npos)
	{
		port = atoi(host.substr(colonPos + 1).c_str());
		host = host.substr(0, colonPos);
		std::cout << "DEBUG: Extracted host=" << host << ", port=" << port << std::endl;
	}
	
	// Find the appropriate server configuration
	const ServerConfig* server = config.findServer(host, port, host);
	
	if (!server)
	{
	    std::cout << "DEBUG: No matching server configuration found" << std::endl;
		response.setStatus(404);
		response.setBody(config.getDefaultErrorPage(404));
		return response;
	}
	
	std::cout << "DEBUG: Found matching server for " << host 
	    << ":" << port << std::endl;
	
	// Find the appropriate location configuration
	const LocationConfig* location = findLocation(*server);
	
	if (!location)
	{
	    std::cout << "DEBUG: No matching location configuration found for " 
	        << _path << std::endl;
		response.setStatus(404);
		response.setBody(config.getDefaultErrorPage(404));
		return response;
	}
	
	std::cout << "DEBUG: Found matching location: " << location->path << std::endl;
	
	// Check if method is allowed
	if (!location->allowedMethods.empty() && 
		location->allowedMethods.find(_method) == location->allowedMethods.end())
	{
	    std::cout << "DEBUG: Method " << _method << " not allowed" << std::endl;
		response.setStatus(405);
		response.setBody(config.getDefaultErrorPage(405));
		return response;
	}
	
	// Handle redirections
	if (!location->redirect.empty())
	{
	    std::cout << "DEBUG: Redirecting to " << location->redirect << std::endl;
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
		std::cout << "DEBUG: Unsupported method " << _method << std::endl;
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
	std::cout << "DEBUG: Handling GET request for " << _path << std::endl;
	
	// Build the full file path
	std::string fullPath = location.root + _path;
	
	// Check for path traversal attacks
	if (!isPathSafe(_path, location.root))
	{
		std::cout << "DEBUG: Path traversal detected: " << _path << std::endl;
		response.setStatus(403);
		response.setBody(config.getDefaultErrorPage(403));
		return response;
	}
	
	// Check if this is a CGI request
	if (CgiHandler::isCgiFile(fullPath, location))
	{
		std::cout << "DEBUG: Detected CGI file, delegating to CGI handler" << std::endl;
		return handleCgi(location, response, config);
	}
	
	// If path ends with '/', try to serve index file
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
		std::cout << "DEBUG: File not found: " << fullPath << std::endl;
		
		// If autoindex is enabled and it's a directory, show directory listing
		if (location.autoindex && !_path.empty() && _path[_path.length() - 1] == '/')
		{
			return generateDirectoryListing(location, response);
		}
		
		response.setStatus(404);
		response.setBody(config.getDefaultErrorPage(404));
		return response;
	}
	
	// Read file content
	std::string content((std::istreambuf_iterator<char>(file)), 
	                   std::istreambuf_iterator<char>());
	file.close();
	
	std::cout << "DEBUG: Successfully read " << content.size() 
	    << " bytes from " << fullPath << std::endl;
	
	// Set response
	response.setStatus(200);
	response.setBody(content);
	response.addHeader("Content-Type", getMimeType(fullPath));
	
	return response;
}

/**
 * Handle POST request for file uploads
 */
HttpResponse	HttpRequest::handlePost(const LocationConfig& location, 
	HttpResponse& response, const Config& config)
{
	std::cout << "DEBUG: Handling POST request for " << _path << std::endl;
	
	// Build the full file path to check for CGI
	std::string fullPath = location.root + _path;
	
	// Check if this is a CGI request
	if (CgiHandler::isCgiFile(fullPath, location))
	{
		std::cout << "DEBUG: Detected CGI file, delegating to CGI handler" << std::endl;
		return handleCgi(location, response, config);
	}
	
	// Check if upload is allowed for this location
	if (location.uploadStore.empty())
	{
		std::cout << "DEBUG: Upload not allowed for this location" << std::endl;
		response.setStatus(403);
		response.setBody(config.getDefaultErrorPage(403));
		return response;
	}
	
	// Parse Content-Type header to handle multipart/form-data
	std::string contentType = getHeader("Content-Type");
	
	if (contentType.find("multipart/form-data") != std::string::npos)
	{
		return handleFileUpload(location, response, config);
	}
	else if (contentType.find("application/x-www-form-urlencoded") != std::string::npos)
	{
		return handleFormData(location, response, config);
	}
	else
	{
		// Simple file upload - treat body as raw file content
		std::ostringstream oss;
		oss << "upload_" << time(NULL);
		std::string filename = oss.str();
		std::string uploadPath = location.uploadStore + "/" + filename;
		
		// Garante que o diretório de upload existe
		mkdir(location.uploadStore.c_str(), 0777);
		std::ofstream file(uploadPath.c_str(), std::ios::binary);
		if (!file.is_open())
		{
			std::cout << "DEBUG: Failed to create upload file: " << uploadPath << std::endl;
			response.setStatus(500);
			response.setBody(config.getDefaultErrorPage(500));
			return response;
		}
		
		file.write(_body.c_str(), _body.size());
		file.close();
		
		std::cout << "DEBUG: Successfully uploaded " << _body.size() 
		    << " bytes to " << uploadPath << std::endl;
		
		response.setStatus(201);
		std::ostringstream sizeOss;
		sizeOss << _body.size();
		response.setBody("<html><body><h1>Upload Successful</h1>"
		                "<p>File uploaded as: " + filename + "</p>"
		                "<p>Size: " + sizeOss.str() + " bytes</p>"
		                "</body></html>");
		response.addHeader("Content-Type", "text/html");
	}
	
	return response;
}

/**
 * Handle DELETE request for file deletion
 */
HttpResponse	HttpRequest::handleDelete(const LocationConfig& location, 
	HttpResponse& response, const Config& config)
{
	std::cout << "DEBUG: Handling DELETE request for " << _path << std::endl;
	
	// Build the full file path
	std::string fullPath = location.root + _path;
	
	// Check for path traversal attacks
	if (!isPathSafe(_path, location.root))
	{
		std::cout << "DEBUG: Path traversal detected: " << _path << std::endl;
		response.setStatus(403);
		response.setBody(config.getDefaultErrorPage(403));
		return response;
	}
	
	// Check if file exists
	std::ifstream file(fullPath.c_str());
	if (!file.good())
	{
		std::cout << "DEBUG: File not found for deletion: " << fullPath << std::endl;
		response.setStatus(404);
		response.setBody(config.getDefaultErrorPage(404));
		return response;
	}
	file.close();
	
	// Try to delete the file
	if (std::remove(fullPath.c_str()) != 0)
	{
		std::cout << "DEBUG: Failed to delete file: " << fullPath << std::endl;
		response.setStatus(500);
		response.setBody(config.getDefaultErrorPage(500));
		return response;
	}
	
	std::cout << "DEBUG: Successfully deleted file: " << fullPath << std::endl;
	
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
	std::cout << "DEBUG: Generating directory listing for " << _path << std::endl;
	
	std::string dirPath = location.root + _path;
	DIR* dir = opendir(dirPath.c_str());
	
	if (!dir)
	{
		std::cout << "DEBUG: Failed to open directory: " << dirPath << std::endl;
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
	std::cout << "DEBUG: Handling multipart file upload" << std::endl;
	
	// Parse boundary from Content-Type header
	std::string contentType = getHeader("Content-Type");
	size_t boundaryPos = contentType.find("boundary=");
	
	if (boundaryPos == std::string::npos)
	{
		std::cout << "DEBUG: No boundary found in multipart data" << std::endl;
		response.setStatus(400);
		response.setBody(config.getDefaultErrorPage(400));
		return response;
	}
	
	std::string boundary = "--" + contentType.substr(boundaryPos + 9);
	std::cout << "DEBUG: Using boundary: " << boundary << std::endl;
	
	// Simple multipart parsing - find file content between boundaries
	size_t startPos = _body.find(boundary);
	if (startPos == std::string::npos)
	{
		response.setStatus(400);
		response.setBody(config.getDefaultErrorPage(400));
		return response;
	}
	
	// Find filename in headers
	size_t filenamePos = _body.find("filename=\"", startPos);
	std::string filename = "uploaded_file";
	
	if (filenamePos != std::string::npos)
	{
		size_t filenameStart = filenamePos + 10;
		size_t filenameEnd = _body.find("\"", filenameStart);
		if (filenameEnd != std::string::npos)
		{
			filename = _body.substr(filenameStart, filenameEnd - filenameStart);
		}
	}
	
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
	
	// Save file
	std::string uploadPath = location.uploadStore + "/" + filename;
	std::ofstream file(uploadPath.c_str(), std::ios::binary);
	
	if (!file.is_open())
	{
		std::cout << "DEBUG: Failed to create upload file: " << uploadPath << std::endl;
		response.setStatus(500);
		response.setBody(config.getDefaultErrorPage(500));
		return response;
	}
	
	file.write(fileContent.c_str(), fileContent.size());
	file.close();
	
	std::cout << "DEBUG: Successfully uploaded " << fileContent.size() 
	    << " bytes to " << uploadPath << std::endl;
	
	response.setStatus(201);
	std::ostringstream sizeOss;
	sizeOss << fileContent.size();
	response.setBody("<html><body><h1>File Upload Successful</h1>"
	                "<p>Filename: " + filename + "</p>"
	                "<p>Size: " + sizeOss.str() + " bytes</p>"
	                "</body></html>");
	response.addHeader("Content-Type", "text/html");
	
	return response;
}

/**
 * Handle application/x-www-form-urlencoded data
 */
HttpResponse	HttpRequest::handleFormData(const LocationConfig& location, 
	HttpResponse& response, const Config& config)
{
	(void)location; // Unused parameter
	(void)config;   // Unused parameter
	
	std::cout << "DEBUG: Handling form data" << std::endl;
	
	response.setStatus(200);
	response.setBody("<html><body><h1>Form Data Received</h1>"
	                "<p>Form processing not yet implemented.</p>"
	                "<p>Received " + _body + "</p>"
	                "</body></html>");
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
	
	std::cout << "DEBUG: Handling CGI request for " << _path << std::endl;
	
	// Build the full script path
	std::string scriptPath = location.root + _path;
	
	// Check for path traversal attacks
	if (!isPathSafe(_path, location.root))
	{
		std::cout << "DEBUG: Path traversal detected: " << _path << std::endl;
		response.setStatus(403);
		response.setBody("Forbidden: Path traversal detected");
		return response;
	}
	
	// Check if it's a CGI file
	if (!CgiHandler::isCgiFile(scriptPath, location))
	{
		std::cout << "DEBUG: File is not a CGI script: " << scriptPath << std::endl;
		response.setStatus(403);
		response.setBody("Forbidden: Not a CGI script");
		return response;
	}
	
	// Use CgiHandler to execute the script
	CgiHandler cgiHandler;
	return cgiHandler.handleCgiRequest(*this, location, scriptPath);
}