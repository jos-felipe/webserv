/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: josfelip <josfelip@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/26 14:30:22 by josfelip          #+#    #+#             */
/*   Updated: 2025/04/01 10:43:16 by josfelip         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_HPP
# define SOCKET_HPP

# include <string>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>

/**
 * @class Socket
 * @brief Wrapper for socket operations
 * 
 * This class encapsulates socket functionality, providing a clean
 * object-oriented interface for socket operations.
 */
class Socket
{
private:
	int				_fd;
	std::string		_host;
	int				_port;
	struct sockaddr_in	_addr;
	bool			_bound;
	bool			_listening;
	
	/**
	 * Initialize the address structure
	 */
	void			initAddress(void);

public:
	/**
	 * Default constructor
	 */
	Socket(void);
	
	/**
	 * Constructor with host and port
	 */
	Socket(const std::string& host, int port);
	
	/**
	 * Constructor with existing socket file descriptor
	 */
	Socket(int fd, const struct sockaddr_in& addr);
	
	/**
	 * Copy constructor
	 */
	Socket(const Socket& other);
	
	/**
	 * Destructor (closes socket)
	 */
	~Socket(void);
	
	/**
	 * Assignment operator
	 */
	Socket&			operator=(const Socket& other);
	
	/**
	 * Bind the socket to its address
	 */
	void			bind(void);
	
	/**
	 * Set the socket to listen for connections
	 */
	void			listen(int backlog = 10);
	
	/**
	 * Accept a new connection
	 */
	Socket			accept(void);
	
	/**
	 * Set the socket to non-blocking mode
	 */
	void			setNonBlocking(void);
	
	/**
	 * Send data on the socket
	 */
	ssize_t			send(const void* buffer, size_t length);
	
	/**
	 * Receive data from the socket
	 */
	ssize_t			recv(void* buffer, size_t length);
	
	/**
	 * Close the socket
	 */
	void			close(void);
	
	/**
	 * Get the socket file descriptor
	 */
	int				getFd(void) const;
	
	/**
	 * Get the host
	 */
	const std::string&	getHost(void) const;
	
	/**
	 * Get the port
	 */
	int				getPort(void) const;
};

#endif