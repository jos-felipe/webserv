/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: josfelip <josfelip@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/26 12:45:21 by josfelip          #+#    #+#             */
/*   Updated: 2025/03/26 12:45:21 by josfelip         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <map>
# include <vector>
# include <string>
# include <sys/time.h>
# include "Config.hpp"
# include "Socket.hpp"
# include "HttpRequest.hpp"
# include "HttpResponse.hpp"

/**
 * @class Server
 * @brief Main HTTP server class that handles connections and request processing
 * 
 * This class is responsible for initializing server sockets based on configuration,
 * accepting new connections, processing HTTP requests, and sending responses.
 * It implements a non-blocking I/O model using poll() as required by the subject.
 */
class Server
{
private:
	const Config&				_config;
	std::vector<Socket>			_listenSockets;
	std::map<int, Socket>		_clientSockets;
	std::map<int, HttpRequest>	_requests;
	std::map<int, HttpResponse>	_responses;
	fd_set						_readFds;
	fd_set						_writeFds;
	fd_set						_errorFds;
	int							_maxFd;
	
	/**
	 * Initialize listening sockets based on configuration
	 */
	void			initializeSockets(void);
	
	/**
	 * Accept new client connections
	 */
	void			acceptConnections(void);
	
	/**
	 * Handle client requests
	 */
	void			handleRequests(void);
	
	/**
	 * Send responses to clients
	 */
	void			sendResponses(void);
	
	/**
	 * Check for and remove timed out connections
	 */
	void			checkTimeouts(void);

public:
	/**
	 * Constructor with configuration
	 */
	Server(const Config& config);
	
	/**
	 * Destructor
	 */
	~Server(void);
	
	/**
	 * Start the server
	 */
	void			start(void);
	
	/**
	 * Run one iteration of the server loop
	 */
	void			run(void);
	
	/**
	 * Stop the server
	 */
	void			stop(void);
};

#endif
