/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: josfelip <josfelip@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/26 14:40:18 by josfelip          #+#    #+#             */
/*   Updated: 2025/04/02 14:10:59 by josfelip         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <stdexcept>

/*
** SocketImpl implementation
*/

/**
 * Default constructor
 */
Socket::SocketImpl::SocketImpl(void) : _fd(-1), _host(""), _port(0),
	_bound(false), _listening(false), _refCount(1)
{
	memset(&_addr, 0, sizeof(_addr));
}

/**
 * Constructor with host and port
 */
Socket::SocketImpl::SocketImpl(const std::string& host, int port) : 
	_host(host), _port(port), _bound(false), _listening(false), _refCount(1)
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
Socket::SocketImpl::SocketImpl(int fd, const struct sockaddr_in& addr) : 
	_fd(fd), _addr(addr), _bound(true), _listening(false), _refCount(1)
{
	_host = inet_ntoa(addr.sin_addr);
	_port = ntohs(addr.sin_port);
}

/**
 * Destructor
 */
Socket::SocketImpl::~SocketImpl(void)
{
	close();
}

/**
 * Initialize the address structure
 */
void	Socket::SocketImpl::initAddress(void)
{
	_addr.sin_family = AF_INET;
	_addr.sin_port = htons(_port);
	
	if (_host == "0.0.0.0" || _host.empty())
		_addr.sin_addr.s_addr = INADDR_ANY;
	else
		_addr.sin_addr.s_addr = inet_addr(_host.c_str());
}

/**
 * Add a reference
 */
void	Socket::SocketImpl::addRef(void)
{
	_refCount++;
}

/**
 * Release a reference, returns true if this was the last reference
 */
bool	Socket::SocketImpl::release(void)
{
	--_refCount;
	return (_refCount <= 0);
}

/**
 * Bind the socket to its address
 */
void	Socket::SocketImpl::bind(void)
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
void	Socket::SocketImpl::listen(int backlog)
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
int		Socket::SocketImpl::accept(struct sockaddr_in& clientAddr)
{
	socklen_t clientLen = sizeof(clientAddr);
	
	int clientFd = ::accept(_fd, (struct sockaddr*)&clientAddr, &clientLen);
	
	if (clientFd < 0)
		throw std::runtime_error("Failed to accept connection");
		
	return clientFd;
}

/**
 * Set the socket to non-blocking mode
 */
void	Socket::SocketImpl::setNonBlocking(void)
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
ssize_t		Socket::SocketImpl::send(const void* buffer, size_t length)
{
	return ::send(_fd, buffer, length, 0);
}

/**
 * Receive data from the socket
 */
ssize_t		Socket::SocketImpl::recv(void* buffer, size_t length)
{
	return ::recv(_fd, buffer, length, 0);
}

/**
 * Close the socket
 */
void	Socket::SocketImpl::close(void)
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
int		Socket::SocketImpl::getFd(void) const
{
	return _fd;
}

/**
 * Get the host
 */
const std::string&	Socket::SocketImpl::getHost(void) const
{
	return _host;
}

/**
 * Get the port
 */
int		Socket::SocketImpl::getPort(void) const
{
	return _port;
}

/*
** Socket public interface implementation
*/

/**
 * Default constructor
 */
Socket::Socket(void) : _impl(new SocketImpl())
{
}

/**
 * Constructor with host and port
 */
Socket::Socket(const std::string& host, int port) : 
	_impl(new SocketImpl(host, port))
{
}

/**
 * Constructor with existing socket file descriptor
 */
Socket::Socket(int fd, const struct sockaddr_in& addr) :
	_impl(new SocketImpl(fd, addr))
{
}

/**
 * Copy constructor - implements reference counting
 */
Socket::Socket(const Socket& other) : _impl(other._impl)
{
	if (_impl)
		_impl->addRef();
}

/**
 * Destructor - releases the reference
 */
Socket::~Socket(void)
{
	if (_impl && _impl->release())
		delete _impl;
}

/**
 * Assignment operator - implements reference counting
 */
Socket&	Socket::operator=(const Socket& other)
{
	if (this != &other)
	{
		// Release current implementation if it exists
		if (_impl && _impl->release())
			delete _impl;
			
		// Assign and add reference to new implementation
		_impl = other._impl;
		if (_impl)
			_impl->addRef();
	}
	
	return *this;
}

/**
 * Bind the socket to its address
 */
void	Socket::bind(void)
{
	if (_impl)
		_impl->bind();
}

/**
 * Set the socket to listen for connections
 */
void	Socket::listen(int backlog)
{
	if (_impl)
		_impl->listen(backlog);
}

/**
 * Accept a new connection
 */
Socket	Socket::accept(void)
{
	if (!_impl)
		throw std::runtime_error("No socket implementation");
		
	struct sockaddr_in clientAddr;
	int clientFd = _impl->accept(clientAddr);
	
	return Socket(clientFd, clientAddr);
}

/**
 * Set the socket to non-blocking mode
 */
void	Socket::setNonBlocking(void)
{
	if (_impl)
		_impl->setNonBlocking();
}

/**
 * Send data on the socket
 */
ssize_t	Socket::send(const void* buffer, size_t length)
{
	if (!_impl)
		return -1;
		
	return _impl->send(buffer, length);
}

/**
 * Receive data from the socket
 */
ssize_t	Socket::recv(void* buffer, size_t length)
{
	if (!_impl)
		return -1;
		
	return _impl->recv(buffer, length);
}

/**
 * Close the socket
 */
void	Socket::close(void)
{
	if (_impl)
		_impl->close();
}

/**
 * Get the socket file descriptor
 */
int	Socket::getFd(void) const
{
	if (!_impl)
		return -1;
		
	return _impl->getFd();
}

/**
 * Get the host
 */
const std::string&	Socket::getHost(void) const
{
	static const std::string empty;
	if (!_impl)
		return empty;
		
	return _impl->getHost();
}

/**
 * Get the port
 */
int	Socket::getPort(void) const
{
	if (!_impl)
		return 0;
		
	return _impl->getPort();
}