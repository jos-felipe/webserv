/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asanni <asanni@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/26 14:20:15 by josfelip          #+#    #+#             */
/*   Updated: 2025/07/13 15:58:11 by asanni           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"
#include <sstream>
#include <ctime>
#include <iostream>
#include <cstring>
#include <cerrno>


HttpResponse::HttpResponse(Logger* logger)
	: _statusCode(200), _statusText("OK"), _bytesSent(0), _keepAlive(true), _logger(logger)
{
}

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
	_keepAlive(other._keepAlive),
	 _logger(other._logger) 
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
	if (_logger)
	{
		std::ostringstream oss;
		oss << "DEBUG: Generating raw response for status " << _statusCode;
		_logger->debug(oss.str());
	}

	std::ostringstream oss;
	oss << "HTTP/1.1 " << _statusCode << " " << _statusText << "\r\n";

	if (_headers.find("Date") == _headers.end())
		_headers["Date"] = getFormattedDate();

	if (_headers.find("Content-Length") == _headers.end())
	{
		std::ostringstream lengthStr;
		lengthStr << _body.length();
		_headers["Content-Length"] = lengthStr.str();
	}

	if (_headers.find("Connection") == _headers.end())
		_headers["Connection"] = _keepAlive ? "keep-alive" : "close";

	if (_body.length() > 0 && _headers.find("Content-Type") == _headers.end())
		_headers["Content-Type"] = "text/html";

	if (_headers.find("Server") == _headers.end())
		_headers["Server"] = "WebServ/0.1";

	for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
		it != _headers.end(); ++it)
	{
		if (_logger)
		{
			std::ostringstream ossHeader;
			ossHeader << "DEBUG: Adding header: " << it->first << ": " << it->second;
			_logger->debug(ossHeader.str());
		}
		oss << it->first << ": " << it->second << "\r\n";
	}

	oss << "\r\n";

	if (!_body.empty())
	{
		if (_logger)
		{
			std::ostringstream ossBody;
			ossBody << "DEBUG: Adding body of " << _body.length() << " bytes";
			_logger->debug(ossBody.str());
		}
		oss << _body;
	}

	_rawResponse = oss.str();

	if (_logger)
	{
		std::string firstPart = _rawResponse.substr(0, std::min(_rawResponse.length(),
			static_cast<size_t>(200)));
		std::ostringstream ossDebug;
		ossDebug << "DEBUG: Raw response (" << _rawResponse.length() << " bytes):\n" << firstPart;
		if (_rawResponse.length() > 200)
			ossDebug << "...";
		_logger->debug(ossDebug.str());
	}
}

std::string	HttpResponse::getFormattedDate(void)
{
	char buffer[100];
	time_t now = time(0);
	struct tm* tm = gmtime(&now);

	strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", tm);
	return std::string(buffer);
}

void	HttpResponse::setStatus(int statusCode)
{
	_statusCode = statusCode;
	_statusText = getStatusText(statusCode);

	if (_logger)
	{
		std::ostringstream oss;
		oss << "DEBUG: Response status set to " << _statusCode << " " << _statusText;
		_logger->debug(oss.str());
	}
}

void	HttpResponse::addHeader(const std::string& name, const std::string& value)
{
	if (_logger)
	{
		std::ostringstream oss;
		oss << "DEBUG: Adding header: " << name << ": " << value;
		_logger->debug(oss.str());
	}
	_headers[name] = value;
}

void	HttpResponse::setBody(const std::string& body)
{
	if (_logger)
	{
		std::ostringstream oss;
		oss << "DEBUG: Setting body with " << body.length() << " bytes";
		_logger->debug(oss.str());
	}
	_body = body;
}

void	HttpResponse::setKeepAlive(bool keepAlive)
{
	if (_logger)
	{
		std::ostringstream oss;
		oss << "DEBUG: Setting keepAlive to " << (keepAlive ? "true" : "false");
		_logger->debug(oss.str());
	}
	_keepAlive = keepAlive;
}

bool	HttpResponse::shouldKeepAlive(void) const
{
	return _keepAlive;
}

bool	HttpResponse::send(Socket& clientSocket)
{
	if (_rawResponse.empty())
	{
		if (_logger)
			_logger->debug("DEBUG: No raw response yet, generating now");
		generateRawResponse();
	}

	size_t remaining = _rawResponse.length() - _bytesSent;

	if (remaining == 0)
	{
		if (_logger)
			_logger->debug("DEBUG: No bytes remaining to send, response complete");
		return true;
	}

	if (_logger)
	{
		std::ostringstream oss;
		oss << "DEBUG: Sending " << remaining << " remaining bytes";
		_logger->debug(oss.str());
	}

	ssize_t bytesSent = clientSocket.send(_rawResponse.c_str() + _bytesSent, remaining);

	if (bytesSent < 0)
	{
		if (_logger)
		{
			std::ostringstream oss;
			oss << "DEBUG: Send failed with error: " << strerror(errno);
			_logger->debug(oss.str());
		}
		throw std::runtime_error("Failed to send response: " + std::string(strerror(errno)));
	}

	if (_logger)
	{
		std::ostringstream oss;
		oss << "DEBUG: Successfully sent " << bytesSent << " bytes";
		_logger->debug(oss.str());
	}

	_bytesSent += bytesSent;

	bool complete = (_bytesSent == _rawResponse.length());

	if (_logger)
	{
		std::ostringstream oss;
		oss << "DEBUG: Response sending is " << (complete ? "complete" : "incomplete")
			<< " (" << _bytesSent << "/" << _rawResponse.length() << " bytes)";
		_logger->debug(oss.str());
	}

	return complete;
}
