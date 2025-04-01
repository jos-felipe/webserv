/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: josfelip <josfelip@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/26 14:10:31 by josfelip          #+#    #+#             */
/*   Updated: 2025/04/01 10:40:25 by josfelip         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_RESPONSE_HPP
# define HTTP_RESPONSE_HPP

# include <string>
# include <map>
# include <vector>
# include "Socket.hpp"

/**
 * @class HttpResponse
 * @brief Represents an HTTP response
 * 
 * This class handles creating and sending HTTP responses, including
 * status line, headers, and body.
 */
class HttpResponse
{
private:
	int								_statusCode;
	std::string						_statusText;
	std::map<std::string, std::string>	_headers;
	std::string						_body;
	std::string						_rawResponse;
	size_t							_bytesSent;
	bool							_keepAlive;
	
	/**
	 * Get the text description for a status code
	 */
	std::string						getStatusText(int statusCode);
	
	/**
	 * Generate the raw HTTP response from components
	 */
	void							generateRawResponse(void);
	
	/**
	 * Get a formatted date string for HTTP headers
	 */
	std::string						getFormattedDate(void);

public:
	/**
	 * Default constructor
	 */
	HttpResponse(void);
	
	/**
	 * Copy constructor
	 */
	HttpResponse(const HttpResponse& other);
	
	/**
	 * Destructor
	 */
	~HttpResponse(void);
	
	/**
	 * Assignment operator
	 */
	HttpResponse&		operator=(const HttpResponse& other);
	
	/**
	 * Set the response status code
	 */
	void							setStatus(int statusCode);
	
	/**
	 * Add a header to the response
	 */
	void							addHeader(const std::string& name, 
									const std::string& value);
	
	/**
	 * Set the response body
	 */
	void							setBody(const std::string& body);
	
	/**
	 * Set whether to keep the connection alive
	 */
	void							setKeepAlive(bool keepAlive);
	
	/**
	 * Check if the connection should be kept alive
	 */
	bool							shouldKeepAlive(void) const;
	
	/**
	 * Send the response to a client socket
	 * Returns true when the response has been fully sent
	 */
	bool							send(Socket& clientSocket);
};

#endif