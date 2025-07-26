/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asanni <asanni@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/26 12:45:21 by josfelip          #+#    #+#             */
/*   Updated: 2025/07/26 15:10:48 by asanni           ###   ########.fr       */
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
# include "Logger.hpp"

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
	Logger						_logger; 
	std::vector<Socket>			_listenSockets;
	std::map<int, Socket>		_clientSockets;
	std::map<int, HttpRequest>	_requests;
	std::map<int, HttpResponse>	_responses;
	fd_set						_readFds;
	fd_set						_writeFds;
	fd_set						_errorFds;
	int								_maxFd;
	
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
	 * Constructor with configuration and logger
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
