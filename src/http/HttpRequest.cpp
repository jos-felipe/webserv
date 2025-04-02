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
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <cerrno>

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
	
	// For now, just return a simple success response
	std::cout << "DEBUG: Generating simple success response" << std::endl;
	response.setStatus(200);
	response.setBody("<html><body><h1>Hello from WebServ!</h1>"
	                 "<p>Your request was processed successfully.</p>"
	                 "<p>Method: " + _method + "</p>"
	                 "<p>Path: " + _path + "</p>"
	                 "</body></html>");
	response.addHeader("Content-Type", "text/html");
	
	return response;
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