/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: josfelip <josfelip@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/26 12:55:47 by josfelip          #+#    #+#             */
/*   Updated: 2025/04/02 17:09:36 by josfelip         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <iostream>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>

/**
 * Default constructor initializes an empty server
 */
Server::Server(void) : _config(NULL), _maxFd(0)
{
	FD_ZERO(&_readFds);
	FD_ZERO(&_writeFds);
	FD_ZERO(&_errorFds);
}

/**
 * Constructor initializes server with provided configuration
 */
Server::Server(const Config& config) : _config(&config), _maxFd(0)
{
	FD_ZERO(&_readFds);
	FD_ZERO(&_writeFds);
	FD_ZERO(&_errorFds);
}

/**
 * Private copy constructor - not implemented to prevent copying
 */
Server::Server(const Server& other) : _config(other._config)
{
	// Not implemented - copying a server with active connections
	// is not allowed as it would duplicate file descriptors
}

/**
 * Private assignment operator - not implemented to prevent assignment
 */
Server&	Server::operator=(const Server& other)
{
	// Not implemented - server assignment is not allowed
	(void)other;
	return *this;
}

/**
 * Destructor cleans up resources
 */
Server::~Server(void)
{
	stop();
}

/**
 * Initialize server sockets for all configured hosts/ports
 */
void	Server::initializeSockets(void)
{
	if (!_config)
		throw std::runtime_error("No configuration provided for server");
		
	std::vector<ServerConfig> servers = _config->getServers();
	
	for (std::vector<ServerConfig>::iterator it = servers.begin();
		it != servers.end(); ++it)
	{
		try
		{
			Socket socket(it->host, it->port);
			socket.setNonBlocking();
			socket.bind();
			socket.listen();
			
			_listenSockets.push_back(socket);
			int fd = socket.getFd();
			
			FD_SET(fd, &_readFds);
			
			if (fd > _maxFd)
				_maxFd = fd;
				
			std::cout << "Server listening on " << it->host << ":" 
				<< it->port << std::endl;
		}
		catch (const std::exception& e)
		{
			std::cerr << "Failed to initialize socket on " << it->host 
				<< ":" << it->port << " - " << e.what() << std::endl;
		}
	}
	
	if (_listenSockets.empty())
		throw std::runtime_error("No valid listening sockets initialized");
}

/**
 * Accept new client connections on listening sockets
 */
void Server::acceptConnections(void)
{
    for (std::vector<Socket>::iterator it = _listenSockets.begin();
        it != _listenSockets.end(); ++it)
    {
        int listenFd = it->getFd();
        
        if (FD_ISSET(listenFd, &_readFds))
        {
            try
            {
                Socket clientSocket = it->accept();
                // Check if accept returned a valid socket
                if (clientSocket.getFd() >= 0)
                {
                    clientSocket.setNonBlocking();
                    
                    int clientFd = clientSocket.getFd();
                    _clientSockets[clientFd] = clientSocket;
                    
                    FD_SET(clientFd, &_readFds);
                    
                    if (clientFd > _maxFd)
                        _maxFd = clientFd;
                        
                    std::cout << "New connection accepted: fd " << clientFd << std::endl;
                }
            }
            catch (const std::exception& e)
            {
                std::cerr << "Failed to accept connection: " 
                    << e.what() << std::endl;
            }
        }
    }
}

/**
 * Handle client requests by reading from ready sockets
 */
void	Server::handleRequests(void)
{
	if (!_config)
		return;
		
	std::vector<int> toRemove;
	
	for (std::map<int, Socket>::iterator it = _clientSockets.begin();
		it != _clientSockets.end(); ++it)
	{
		int clientFd = it->first;
		
		if (FD_ISSET(clientFd, &_readFds))
		{
			try
			{
				if (!_requests.count(clientFd))
					_requests[clientFd] = HttpRequest();
					
				HttpRequest& request = _requests[clientFd];
				
				if (request.read(it->second))
				{
					// Request is complete, process it
					HttpResponse response = request.process(*_config);
					_responses[clientFd] = response;
					
					// Switch to writing mode
					FD_CLR(clientFd, &_readFds);
					FD_SET(clientFd, &_writeFds);
				}
			}
			catch (const std::exception& e)
			{
				std::cerr << "Error handling request on fd " << clientFd 
					<< ": " << e.what() << std::endl;
				toRemove.push_back(clientFd);
			}
		}
	}
	
	// Clean up any failed connections
	for (std::vector<int>::iterator it = toRemove.begin();
		it != toRemove.end(); ++it)
	{
		FD_CLR(*it, &_readFds);
		FD_CLR(*it, &_writeFds);
		_clientSockets.erase(*it);
		_requests.erase(*it);
		_responses.erase(*it);
		close(*it);
	}
}

/**
 * Send responses to clients with ready write descriptors
 */
void	Server::sendResponses(void)
{
	std::vector<int> toRemove;
	
	for (std::map<int, HttpResponse>::iterator it = _responses.begin();
		it != _responses.end(); ++it)
	{
		int clientFd = it->first;
		
		// Validate file descriptor before using it
		if (clientFd < 0 || clientFd >= FD_SETSIZE) {
			std::cerr << "Error: Invalid file descriptor " << clientFd << std::endl;
			toRemove.push_back(clientFd);
			continue;
		}
		
		if (FD_ISSET(clientFd, &_writeFds))
		{
			try
			{
				HttpResponse& response = it->second;
				
				// Check if client socket exists before accessing it
				if (_clientSockets.find(clientFd) == _clientSockets.end()) {
					std::cerr << "Error: Client socket not found for fd " 
						<< clientFd << std::endl;
					toRemove.push_back(clientFd);
					continue;
				}
				
				if (response.send(_clientSockets[clientFd]))
				{
					// Response fully sent, either keep-alive or close
					if (response.shouldKeepAlive())
					{
						// Reset for new request
						_requests.erase(clientFd);
						_responses.erase(clientFd);
						FD_CLR(clientFd, &_writeFds);
						FD_SET(clientFd, &_readFds);
					}
					else
					{
						toRemove.push_back(clientFd);
					}
				}
			}
			catch (const std::exception& e)
			{
				std::cerr << "Error sending response on fd " << clientFd 
					<< ": " << e.what() << std::endl;
				toRemove.push_back(clientFd);
			}
		}
	}
	
	// Clean up connections we're done with
	for (std::vector<int>::iterator it = toRemove.begin();
		it != toRemove.end(); ++it)
	{
		FD_CLR(*it, &_readFds);
		FD_CLR(*it, &_writeFds);
		_clientSockets.erase(*it);
		_requests.erase(*it);
		_responses.erase(*it);
		close(*it);
		std::cout << "Connection closed: fd " << *it << std::endl;
	}
}

/**
 * Check for and remove idle connections that have timed out
 */
void	Server::checkTimeouts(void)
{
	// Implementation would check timestamps of connections
	// and remove those that exceed timeout threshold
}

/**
 * Start the server by initializing sockets
 */
void	Server::start(void)
{
	initializeSockets();
	std::cout << "Server started successfully" << std::endl;
}

/**
 * Run one iteration of the server event loop
 */
void	Server::run(void)
{
	fd_set readFdsCopy = _readFds;
	fd_set writeFdsCopy = _writeFds;
	fd_set errorFdsCopy = _errorFds;
	
	struct timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	
	int activity = select(_maxFd + 1, &readFdsCopy, &writeFdsCopy, 
		&errorFdsCopy, &timeout);
	
	if (activity < 0)
	{
		if (errno != EINTR) // Ignore if interrupted by signal
			std::cerr << "Select error: " << strerror(errno) << std::endl;
		return;
	}
	
	// Updated to use the modified fd sets from select
	// This is a critical fix to avoid checking the wrong file descriptors
	_readFds = readFdsCopy;
	_writeFds = writeFdsCopy;
	_errorFds = errorFdsCopy;
	
	// Process I/O events
	acceptConnections();
	handleRequests();
	sendResponses();
	checkTimeouts();
}

/**
 * Stop the server and clean up resources
 */
void	Server::stop(void)
{
	for (std::map<int, Socket>::iterator it = _clientSockets.begin();
		it != _clientSockets.end(); ++it)
	{
		close(it->first);
	}
	
	for (std::vector<Socket>::iterator it = _listenSockets.begin();
		it != _listenSockets.end(); ++it)
	{
		close(it->getFd());
	}
	
	_clientSockets.clear();
	_listenSockets.clear();
	_requests.clear();
	_responses.clear();
	
	std::cout << "Server stopped" << std::endl;
}