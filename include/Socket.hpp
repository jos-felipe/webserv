/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: josfelip <josfelip@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/26 14:30:22 by josfelip          #+#    #+#             */
/*   Updated: 2025/04/02 14:47:21 by josfelip         ###   ########.fr       */
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
 * @brief Wrapper for socket operations with safe copying semantics
 * 
 * This class encapsulates socket functionality using reference counting
 * to safely manage the underlying file descriptor when copied.
 */
class Socket
{
private:
	/**
	 * @class SocketImpl
	 * @brief Private implementation with reference counting
	 * 
	 * This internal class manages the actual socket resources and
	 * implements reference counting to track when the socket can be closed.
	 */
	class SocketImpl
	{
	private:
		int					_fd;
		std::string			_host;
		int					_port;
		struct sockaddr_in	_addr;
		bool				_bound;
		bool				_listening;
		int					_refCount;  // Reference counter
		
	public:
		/**
		 * Default constructor
		 */
		SocketImpl(void);
		
		/**
		 * Constructor with host and port
		 */
		SocketImpl(const std::string& host, int port);
		
		/**
		 * Constructor with existing socket file descriptor
		 */
		SocketImpl(int fd, const struct sockaddr_in& addr);
		
		/**
		 * Destructor
		 */
		~SocketImpl(void);
		
		/**
		 * Initialize the address structure
		 */
		void				initAddress(void);
		
		/**
		 * Add a reference
		 */
		void				addRef(void);
		
		/**
		 * Release a reference, returns true if this was the last reference
		 */
		bool				release(void);
		
		/**
		 * Bind the socket to its address
		 */
		void				bind(void);
		
		/**
		 * Set the socket to listen for connections
		 */
		void				listen(int backlog);
		
		/**
		 * Accept a new connection
		 */
		int					accept(struct sockaddr_in& clientAddr);
		
		/**
		 * Set the socket to non-blocking mode
		 */
		void				setNonBlocking(void);
		
		/**
		 * Send data on the socket
		 */
		ssize_t				send(const void* buffer, size_t length);
		
		/**
		 * Receive data from the socket
		 */
		ssize_t				recv(void* buffer, size_t length);
		
		/**
		 * Close the socket
		 */
		void				close(void);
		
		/**
		 * Get the socket file descriptor
		 */
		int					getFd(void) const;
		
		/**
		 * Get the host
		 */
		const std::string&	getHost(void) const;
		
		/**
		 * Get the port
		 */
		int					getPort(void) const;
	};
	
	SocketImpl*			_impl;  // Pointer to implementation
	
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
	 * Copy constructor - implements reference counting
	 */
	Socket(const Socket& other);
	
	/**
	 * Destructor - releases the reference
	 */
	~Socket(void);
	
	/**
	 * Assignment operator - implements reference counting
	 */
	Socket&				operator=(const Socket& other);
	
	/**
	 * Bind the socket to its address
	 */
	void				bind(void);
	
	/**
	 * Set the socket to listen for connections
	 */
	void				listen(int backlog = 10);
	
	/**
	 * Accept a new connection
	 */
	Socket				accept(void);
	
	/**
	 * Set the socket to non-blocking mode
	 */
	void				setNonBlocking(void);
	
	/**
	 * Send data on the socket
	 */
	ssize_t				send(const void* buffer, size_t length);
	
	/**
	 * Receive data from the socket
	 */
	ssize_t				recv(void* buffer, size_t length);
	
	/**
	 * Close the socket
	 */
	void				close(void);
	
	/**
	 * Get the socket file descriptor
	 */
	int					getFd(void) const;
	
	/**
	 * Get the host
	 */
	const std::string&	getHost(void) const;
	
	/**
	 * Get the port
	 */
	int					getPort(void) const;
};

#endif