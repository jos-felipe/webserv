/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: josfelip <josfelip@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/26 12:45:21 by josfelip          #+#    #+#             */
/*   Updated: 2025/06/10 12:41:42 by josfelip         ###   ########.fr       */
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
	const Config*				_config;  // Changed to pointer to allow default constructor
	std::vector<Socket>			_listenSockets;
	std::map<int, Socket>		_clientSockets;
	std::map<int, HttpRequest>	_requests;
	std::map<int, HttpResponse>	_responses;
	fd_set						_readFds;
	fd_set						_writeFds;
	fd_set						_errorFds;
	int							_maxFd;
	bool						_stopped;  // Flag to indicate if server is stopped
	
	/**
	 * Initialize listening sockets based on configuration
	 */
	void			initializeSockets(void);
	
	/**
	 * Accept new client connections
	 */
	void			acceptConnections(fd_set *readFdsReady);
	
	/**
	 * Handle client requests
	 */
	void			handleRequests(fd_set *readFdsReady);
	
	/**
	 * Send responses to clients
	 */
	void			sendResponses(fd_set *writeFdsReady);
	
		/**
	 * Copy constructor - private to prevent copying
	 */
	Server(const Server& other);
	
	/**
	 * Assignment operator - private to prevent assignment
	 */
	Server&			operator=(const Server& other);

public:
	/**
	 * Default constructor
	 */
	Server(void);
	
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