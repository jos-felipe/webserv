/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: josfelip <josfelip@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/26 14:20:15 by josfelip          #+#    #+#             */
/*   Updated: 2025/04/01 10:40:51 by josfelip         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"
#include <sstream>
#include <ctime>

/**
 * Constructor initializes a default response
 */
HttpResponse::HttpResponse(void) : _statusCode(200), _statusText("OK"), 
	_bytesSent(0), _keepAlive(true)
{
}

/**
 * Copy constructor
 */
HttpResponse::HttpResponse(const HttpResponse& other) :
	_statusCode(other._statusCode),
	_statusText(other._statusText),
	_headers(other._headers),
	_body(other._body),
	_rawResponse(other._rawResponse),
	_bytesSent(other._bytesSent),
	_keepAlive(other._keepAlive)
{
}

/**
 * Destructor
 */
HttpResponse::~HttpResponse(void)
{
}

/**
 * Assignment operator
 */
HttpResponse&	HttpResponse::operator=(const HttpResponse& other)
{
	if (this != &other)
	{
		_statusCode = other._statusCode;
		_statusText = other._statusText;
		_headers = other._headers;
		_body = other._body;
		_rawResponse = other._rawResponse;
		_bytesSent = other._bytesSent;
		_keepAlive = other._keepAlive;
	}
	return *this;
}

/**
 * Get the text description for a status code
 */
std::string	HttpResponse::getStatusText(int statusCode)
{
	switch (statusCode)
	{
		case 100: return "Continue";
		case 101: return "Switching Protocols";
		case 200: return "OK";
		case 201: return "Created";
		case 202: return "Accepted";
		case 204: return "No Content";
		case 206: return "Partial Content";
		case 300: return "Multiple Choices";
		case 301: return "Moved Permanently";
		case 302: return "Found";
		case 303: return "See Other";
		case 304: return "Not Modified";
		case 307: return "Temporary Redirect";
		case 308: return "Permanent Redirect";
		case 400: return "Bad Request";
		case 401: return "Unauthorized";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		case 406: return "Not Acceptable";
		case 408: return "Request Timeout";
		case 409: return "Conflict";
		case 410: return "Gone";
		case 411: return "Length Required";
		case 413: return "Payload Too Large";
		case 414: return "URI Too Long";
		case 415: return "Unsupported Media Type";
		case 416: return "Range Not Satisfiable";
		case 417: return "Expectation Failed";
		case 418: return "I'm a teapot";
		case 422: return "Unprocessable Entity";
		case 429: return "Too Many Requests";
		case 500: return "Internal Server Error";
		case 501: return "Not Implemented";
		case 502: return "Bad Gateway";
		case 503: return "Service Unavailable";
		case 504: return "Gateway Timeout";
		case 505: return "HTTP Version Not Supported";
		default: return "Unknown";
	}
}

/**
 * Generate the raw HTTP response from components
 */
void	HttpResponse::generateRawResponse(void)
{
	std::ostringstream oss;
	
	// Status line
	oss << "HTTP/1.1 " << _statusCode << " " << _statusText << "\r\n";
	
	// Add default headers if not present
	if (_headers.find("Date") == _headers.end())
		_headers["Date"] = getFormattedDate();
		
	if (_headers.find("Content-Length") == _headers.end())
		_headers["Content-Length"] = static_cast<std::ostringstream&>(
			std::ostringstream() << _body.length()).str();
			
	if (_headers.find("Connection") == _headers.end())
		_headers["Connection"] = _keepAlive ? "keep-alive" : "close";
		
	if (_body.length() > 0 && _headers.find("Content-Type") == _headers.end())
		_headers["Content-Type"] = "text/html";
		
	// Add all headers
	for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
		it != _headers.end(); ++it)
	{
		oss << it->first << ": " << it->second << "\r\n";
	}
	
	// Empty line separating headers from body
	oss << "\r\n";
	
	// Add body
	oss << _body;
	
	_rawResponse = oss.str();
}

/**
 * Get a formatted date string for HTTP headers
 */
std::string	HttpResponse::getFormattedDate(void)
{
	char buffer[100];
	time_t now = time(0);
	struct tm* tm = gmtime(&now);
	
	strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", tm);
	return std::string(buffer);
}

/**
 * Set the response status code
 */
void	HttpResponse::setStatus(int statusCode)
{
	_statusCode = statusCode;
	_statusText = getStatusText(statusCode);
}

/**
 * Add a header to the response
 */
void	HttpResponse::addHeader(const std::string& name, const std::string& value)
{
	_headers[name] = value;
}

/**
 * Set the response body
 */
void	HttpResponse::setBody(const std::string& body)
{
	_body = body;
}

/**
 * Set whether to keep the connection alive
 */
void	HttpResponse::setKeepAlive(bool keepAlive)
{
	_keepAlive = keepAlive;
}

/**
 * Check if the connection should be kept alive
 */
bool	HttpResponse::shouldKeepAlive(void) const
{
	return _keepAlive;
}

/**
 * Send the response to a client socket
 * Returns true when the response has been fully sent
 */
bool	HttpResponse::send(Socket& clientSocket)
{
	// Generate raw response if not already done
	if (_rawResponse.empty())
		generateRawResponse();
		
	// Calculate remaining bytes to send
	size_t remaining = _rawResponse.length() - _bytesSent;
	
	if (remaining == 0)
		return true;
		
	// Try to send the remaining data
	ssize_t bytesSent = clientSocket.send(_rawResponse.c_str() + _bytesSent, 
		remaining);
		
	if (bytesSent < 0)
		throw std::runtime_error("Failed to send response");
		
	_bytesSent += bytesSent;
	
	// Check if we've sent everything
	return (_bytesSent == _rawResponse.length());
}