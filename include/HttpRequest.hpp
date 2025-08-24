/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asanni <asanni@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/26 13:35:21 by josfelip          #+#    #+#             */
/*   Updated: 2025/08/09 16:05:48 by asanni           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_REQUEST_HPP
# define HTTP_REQUEST_HPP

# include <string>
# include <map>
# include "Socket.hpp"
# include "Config.hpp"
# include "HttpResponse.hpp"
# include "CgiHandler.hpp"
# include "Logger.hpp"

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
	bool								_connectionError;
	const ServerConfig*					_serverConfig;
	Logger								_logger;
	
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
	
	/**
	 * Handle GET request for static file serving
	 */
	HttpResponse						handleGet(const LocationConfig& location, 
												HttpResponse& response, const Config& config);
	
	HttpResponse handlePost(LocationConfig const &location, 
		HttpResponse &response, Config const &config);
	
	/**
	 * Handle DELETE request for file deletion
	 */
	HttpResponse						handleDelete(const LocationConfig& location, 
												HttpResponse& response, const Config& config);
	
	/**
	 * Get MIME type for file extension
	 */
	std::string							getMimeType(const std::string& filename) const;
	
	/**
	 * Check if path is safe and within allowed directory
	 */
	bool								isPathSafe(const std::string& path, 
												const std::string& root) const;
	
	/**
	 * Generate directory listing for autoindex
	 */
	HttpResponse						generateDirectoryListing(const LocationConfig& location, 
														HttpResponse& response);
	
	/**
	 * Handle multipart/form-data file upload
	 */
	HttpResponse						handleFileUpload(const LocationConfig& location, 
													HttpResponse& response, const Config& config);
	
	HttpResponse handleFormData(HttpResponse &response); 
	
	/**
	 * Handle CGI request execution
	 */
	HttpResponse						handleCgi(const LocationConfig& location, 
												HttpResponse& response, const Config& config);

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
	
	/**
	 * Check if there was a connection error (should close connection)
	 */
	bool								hasConnectionError(void) const;
	
	/**
	 * Set server configuration for size validation during parsing
	 */
	void								setServerConfig(const ServerConfig* serverConfig);
};

#endif
