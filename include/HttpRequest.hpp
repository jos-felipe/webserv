/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: josfelip <josfelip@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/26 13:35:21 by josfelip          #+#    #+#             */
/*   Updated: 2025/04/01 10:35:11 by josfelip         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_REQUEST_HPP
# define HTTP_REQUEST_HPP

# include <string>
# include <map>
# include "Socket.hpp"
# include "Config.hpp"
# include "HttpResponse.hpp"

/**
 * @enum ParseState
 * @brief States for HTTP request parsing
 */
enum ParseState
{
	REQUEST_LINE,
	HEADERS,
	BODY,
	CHUNKED_SIZE,
	CHUNKED_DATA,
	CHUNKED_END,
	COMPLETE,
	ERROR
};

/**
 * @class HttpRequest
 * @brief Handles HTTP request parsing and processing
 * 
 * This class is responsible for parsing incoming HTTP requests,
 * validating them according to the HTTP/1.1 spec, and generating
 * appropriate responses based on the server configuration.
 */
class HttpRequest
{
private:
	std::string							_method;
	std::string							_uri;
	std::string							_httpVersion;
	std::string							_path;
	std::string							_query;
	std::map<std::string, std::string>	_headers;
	std::string							_body;
	
	ParseState							_state;
	std::string							_buffer;
	size_t								_contentLength;
	size_t								_chunkSize;
	bool								_chunked;
	
	/**
	 * Parse the request line (GET /path HTTP/1.1)
	 */
	bool								parseRequestLine(void);
	
	/**
	 * Parse HTTP headers
	 */
	bool								parseHeaders(void);
	
	/**
	 * Parse request body
	 */
	bool								parseBody(void);
	
	/**
	 * Parse chunked encoding size line
	 */
	bool								parseChunkedSize(void);
	
	/**
	 * Parse chunked encoding data
	 */
	bool								parseChunkedData(void);
	
	/**
	 * Find the location configuration for this request
	 */
	const LocationConfig*				findLocation(const ServerConfig& server) const;

public:
	/**
	 * Default constructor
	 */
	HttpRequest(void);
	
	/**
	 * Copy constructor
	 */
	HttpRequest(const HttpRequest& other);
	
	/**
	 * Destructor
	 */
	~HttpRequest(void);
	
	/**
	 * Assignment operator
	 */
	HttpRequest&		operator=(const HttpRequest& other);
	
	/**
	 * Read and parse data from a client socket
	 * Returns true when request is complete
	 */
	bool								read(Socket& clientSocket);
	
	/**
	 * Process the request and generate a response
	 */
	HttpResponse						process(const Config& config);
	
	/**
	 * Get the request method
	 */
	const std::string&					getMethod(void) const;
	
	/**
	 * Get the request URI
	 */
	const std::string&					getUri(void) const;
	
	/**
	 * Get the request HTTP version
	 */
	const std::string&					getHttpVersion(void) const;
	
	/**
	 * Get a specific header value
	 */
	std::string							getHeader(const std::string& name) const;
	
	/**
	 * Get all headers
	 */
	const std::map<std::string, std::string>&	getHeaders(void) const;
	
	/**
	 * Get the request body
	 */
	const std::string&					getBody(void) const;
};

#endif