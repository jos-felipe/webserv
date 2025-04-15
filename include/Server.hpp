/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: josfelip <josfelip@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/26 12:45:21 by josfelip          #+#    #+#             */
/*   Updated: 2025/04/15 15:07:52 by josfelip         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <map>
# include <vector>
# include <string>
# include <sys/time.h>
# include <sys/epoll.h>
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
 * It implements a non-blocking I/O model using epoll() as required by the subject.
 */
class Server
{
private:
	const Config*				_config;  // Changed to pointer to allow default constructor
	std::vector<Socket>			_listenSockets;
	std::map<int, Socket>		_clientSockets;
	std::map<int, HttpRequest>	_requests;
	std::map<int, HttpResponse>	_responses;
	int							_epollFd;
	
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

	void			registerFd(int fd, uint32_t events);
	void			modifyFd(int fd, uint32_t events);
	void			unregisterFd(int fd);
	
	void			setReadable(int fd, bool enable);
	void			setWritable(int fd, bool enable);
	
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