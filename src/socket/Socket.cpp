/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: josfelip <josfelip@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/26 14:40:18 by josfelip          #+#    #+#             */
/*   Updated: 2025/04/01 10:43:34 by josfelip         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <stdexcept>

/**
 * Default constructor creates an uninitialized socket
 */
Socket::Socket(void) : _fd(-1), _host(""), _port(0), _bound(false), 
	_listening(false)
{
	memset(&_addr, 0, sizeof(_addr));
}

/**
 * Constructor creates a socket with specified host and port
 */
Socket::Socket(const std::string& host, int port) : _host(host), _port(port), 
	_bound(false), _listening(false)
{
	memset(&_addr, 0, sizeof(_addr));
	
	// Create the socket
	_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_fd < 0)
		throw std::runtime_error("Failed to create socket");
		
	// Set address reuse
	int opt = 1;
	if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		close();
		throw std::runtime_error("Failed to set socket options");
	}
	
	initAddress();
}

/**
 * Constructor with existing socket file descriptor
 */
Socket::Socket(int fd, const struct sockaddr_in& addr) : _fd(fd), _addr(addr), 
	_bound(true), _listening(false)
{
	_host = inet_ntoa(addr.sin_addr);
	_port = ntohs(addr.sin_port);
}

/**
 * Copy constructor
 */
Socket::Socket(const Socket& other) : _fd(-1)
{
	*this = other;
}

/**
 * Destructor
 */
Socket::~Socket(void)
{
	close();
}

/**
 * Assignment operator
 */
Socket&	Socket::operator=(const Socket& other)
{
	if (this != &other)
	{
		// Close existing socket if open
		close();
		
		// Copy everything except the file descriptor
		_host = other._host;
		_port = other._port;
		_addr = other._addr;
		_bound = other._bound;
		_listening = other._listening;
		
		// Create a new socket (don't share file descriptors)
		if (other._fd >= 0)
		{
			_fd = socket(AF_INET, SOCK_STREAM, 0);
			if (_fd < 0)
				throw std::runtime_error("Failed to create socket in assignment");
				
			// Set address reuse
			int opt = 1;
			if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
			{
				close();
				throw std::runtime_error("Failed to set socket options");
			}
		}
	}
	
	return *this;
}

/**
 * Initialize the address structure
 */
void	Socket::initAddress(void)
{
	_addr.sin_family = AF_INET;
	_addr.sin_port = htons(_port);
	
	if (_host == "0.0.0.0" || _host.empty())
		_addr.sin_addr.s_addr = INADDR_ANY;
	else
		_addr.sin_addr.s_addr = inet_addr(_host.c_str());
}

/**
 * Bind the socket to its address
 */
void	Socket::bind(void)
{
	if (_bound)
		return;
		
	if (::bind(_fd, (struct sockaddr*)&_addr, sizeof(_addr)) < 0)
		throw std::runtime_error("Failed to bind socket");
		
	_bound = true;
}

/**
 * Set the socket to listen for connections
 */
void	Socket::listen(int backlog)
{
	if (!_bound)
		bind();
		
	if (_listening)
		return;
		
	if (::listen(_fd, backlog) < 0)
		throw std::runtime_error("Failed to listen on socket");
		
	_listening = true;
}

/**
 * Accept a new connection
 */
Socket	Socket::accept(void)
{
	struct sockaddr_in clientAddr;
	socklen_t clientLen = sizeof(clientAddr);
	
	int clientFd = ::accept(_fd, (struct sockaddr*)&clientAddr, &clientLen);
	
	if (clientFd < 0)
		throw std::runtime_error("Failed to accept connection");
		
	return Socket(clientFd, clientAddr);
}

/**
 * Set the socket to non-blocking mode
 */
void	Socket::setNonBlocking(void)
{
	int flags = fcntl(_fd, F_GETFL, 0);
	
	if (flags < 0)
		throw std::runtime_error("Failed to get socket flags");
		
	flags |= O_NONBLOCK;
	
	if (fcntl(_fd, F_SETFL, flags) < 0)
		throw std::runtime_error("Failed to set socket to non-blocking");
}

/**
 * Send data on the socket
 */
ssize_t	Socket::send(const void* buffer, size_t length)
{
	return ::send(_fd, buffer, length, 0);
}

/**
 * Receive data from the socket
 */
ssize_t	Socket::recv(void* buffer, size_t length)
{
	return ::recv(_fd, buffer, length, 0);
}

/**
 * Close the socket
 */
void	Socket::close(void)
{
	if (_fd >= 0)
	{
		::close(_fd);
		_fd = -1;
		_bound = false;
		_listening = false;
	}
}

/**
 * Get the socket file descriptor
 */
int	Socket::getFd(void) const
{
	return _fd;
}

/**
 * Get the host
 */
const std::string&	Socket::getHost(void) const
{
	return _host;
}

/**
 * Get the port
 */
int	Socket::getPort(void) const
{
	return _port;
}