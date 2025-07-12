/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asanni <asanni@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/26 12:55:47 by josfelip          #+#    #+#             */
/*   Updated: 2025/07/12 17:24:23 by asanni           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <iostream>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <sstream>
#include "Logger.hpp"

/**
 * Default constructor initializes an empty server
 */
Server::Server(void) : _config(NULL), _maxFd(-1)
{
	FD_ZERO(&_readFds);
	FD_ZERO(&_writeFds);
	FD_ZERO(&_errorFds);
}

/**
 * Constructor initializes server with provided configuration
 */
Server::Server(const Config& config, Logger& logger) : _config(&config), _logger(&logger), _maxFd(-1)
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
			std::ostringstream oss;
            oss << "Server listening on " << it->host << ":" << it->port;
            _logger->info( oss.str());
		}
		catch (const std::exception& e)
		{   
            std::ostringstream oss;
			oss << "Failed to initialize socket on " << it->host 
				<< ":" << it->port << " - " << e.what() << std::endl;
            _logger->error(oss.str());
		}
	}
	
	if (_listenSockets.empty())
		throw std::runtime_error("No valid listening sockets initialized");
}

/**
 * Accept new client connections on listening sockets
 */
void Server::acceptConnections(fd_set *readFdsReady)
{
    for (std::vector<Socket>::iterator it = _listenSockets.begin();
        it != _listenSockets.end(); ++it)
    {
        int listenFd = it->getFd();
        
        if (FD_ISSET(listenFd, readFdsReady))
        {
            std::ostringstream oss;
			oss << "DEBUG: Listen socket " << listenFd 
                << " is ready for accepting" << std::endl;
                _logger->error(oss.str());
                
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
                    std::ostringstream oss;
                    oss << "New connection accepted: fd " << clientFd << std::endl;
                    _logger->info(oss.str());    
                }
            }
            catch (const std::exception& e)
            {
                std::ostringstream oss;
                oss << "Failed to accept connection: " 
                    << e.what() << std::endl;
                _logger->error(oss.str());
            }
        }
    }
}

/**
 * Handle client requests by reading from ready sockets
 */
void	Server::handleRequests(fd_set *readFdsReady)
{
	if (!_config)
		return;
		
	std::vector<int> toRemove;
	
	for (std::map<int, Socket>::iterator it = _clientSockets.begin();
		it != _clientSockets.end(); ++it)
	{
		int clientFd = it->first;
		
		if (FD_ISSET(clientFd, readFdsReady))
		{
            std::ostringstream oss;
		    oss << "DEBUG: Client socket " << clientFd 
            << " is ready for reading" << std::endl;
            _logger->debug(oss.str());
			try
			{
				if (!_requests.count(clientFd))
				{
                    std::ostringstream oss;
                    oss << "DEBUG: Creating new request for fd " 
                    << clientFd << std::endl;
                    _logger->debug(oss.str());
					_requests[clientFd] = HttpRequest();
				}
					
				HttpRequest& request = _requests[clientFd];
				
				bool requestComplete = request.read(it->second);
                    std::ostringstream oss;
                    oss << "DEBUG: Request read returned: " 
                    << (requestComplete ? "COMPLETE" : "INCOMPLETE") << std::endl;
                    _logger->debug(oss.str());
				
				if (requestComplete)
				{
					// Request is complete, process it

                    std::ostringstream oss1;
                    oss1 << "DEBUG: Processing request and generating response" 
                    << std::endl;
                    _logger->debug(oss.str());
					HttpResponse response = request.process(*_config);
					_responses[clientFd] = response;
					
					// Switch to writing mode
                    std::ostringstream oss2;
					oss2 << "DEBUG: Switching socket " << clientFd 
                    << " to write mode" << std::endl;
                    _logger->debug(oss.str());
					FD_CLR(clientFd, &_readFds);
					FD_SET(clientFd, &_writeFds);
				}
			}
			catch (const std::exception& e)
			{
                std::ostringstream oss;
				oss << "Error handling request on fd " << clientFd 
                << ": " << e.what() << std::endl;
                _logger->error(oss.str());
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
void Server::sendResponses(fd_set *writeFdsReady)
{
    std::ostringstream oss;
    oss << "DEBUG: Checking for sockets ready to write" << std::endl;
    _logger->debug(oss.str());
    
    std::vector<int> toRemove;
    std::vector<int> toKeepAlive;  // New vector to track connections to keep alive
    
    for (std::map<int, HttpResponse>::iterator it = _responses.begin();
        it != _responses.end(); ++it)
    {
        int clientFd = it->first;
        
        // Validate file descriptor before using it
        if (clientFd < 0 || clientFd >= FD_SETSIZE) {
            std::ostringstream oss;
            oss << "Error: Invalid file descriptor " << clientFd << std::endl;
            _logger->error(oss.str());
            toRemove.push_back(clientFd);
            continue;
        }
        
        std::ostringstream oss;
        oss << "DEBUG: Checking if socket " << clientFd 
        << " is ready for writing: " 
        << (FD_ISSET(clientFd, writeFdsReady) ? "YES" : "NO") << std::endl;
        _logger->debug(oss.str());
            
        if (FD_ISSET(clientFd, writeFdsReady))
        {
            try
            {
                HttpResponse& response = it->second;
                
                // Check if client socket exists before accessing it
                if (_clientSockets.find(clientFd) == _clientSockets.end()) {
                    std::ostringstream oss;
                    oss << "Error: Client socket not found for fd " 
                    << clientFd << std::endl;
                    _logger->error(oss.str());
                    toRemove.push_back(clientFd);
                    continue;
                }
                std::ostringstream oss;
                oss << "DEBUG: Attempting to send response on fd " 
                << clientFd << std::endl;
                _logger->debug(oss.str());
                    
                if (response.send(_clientSockets[clientFd]))
                {
                    // Response fully sent, either keep-alive or close
                    std::ostringstream oss;
                    oss << "DEBUG: Response fully sent on fd " 
                        << clientFd << std::endl;
                    _logger->debug(oss.str());
                        
                    if (response.shouldKeepAlive())
                    {
                        // Mark for keep-alive processing AFTER we finish iterating
                        std::ostringstream oss;
                        oss << "DEBUG: Marking connection for keep-alive: " 
                        << clientFd << std::endl;
                        _logger->debug(oss.str());
                        toKeepAlive.push_back(clientFd);
                    }
                    else
                    {
                        std::ostringstream oss;
                        oss << "DEBUG: Connection will be closed" << std::endl;
                        _logger->debug(oss.str());
                        toRemove.push_back(clientFd);
                    }
                }
                else
                {
                    std::ostringstream oss;
                    oss << "DEBUG: Response not fully sent yet, "
                    << "will try again later" << std::endl;
                    _logger->debug(oss.str());
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
    
    // Process keep-alive connections AFTER iteration is complete
    for (std::vector<int>::iterator it = toKeepAlive.begin();
         it != toKeepAlive.end(); ++it)
    {
        int clientFd = *it;
        std::ostringstream oss;
        oss << "DEBUG: Keeping connection alive, "
        << "switching " << clientFd << " back to read mode" << std::endl;
        _logger->debug(oss.str());
        
        // Reset for new request
        _requests.erase(clientFd);
        _responses.erase(clientFd);
        FD_CLR(clientFd, &_writeFds);
        FD_SET(clientFd, &_readFds);
    }
    
    // Clean up connections we're done with
    for (std::vector<int>::iterator it = toRemove.begin();
        it != toRemove.end(); ++it)
    {
        int clientFd = *it;
        std::ostringstream oss1;
        oss1 << "DEBUG: Closing connection: " << clientFd << std::endl;
        _logger->debug(oss.str());
        
        FD_CLR(clientFd, &_readFds);
        FD_CLR(clientFd, &_writeFds);
        _clientSockets.erase(clientFd);
        _requests.erase(clientFd);
        _responses.erase(clientFd);
        close(clientFd);
        std::ostringstream oss2;
        oss2 << "Connection closed: fd " << clientFd << std::endl;
        _logger->debug(oss.str());
    }
}
/**
 * Start the server by initializing sockets
 */
void	Server::start(void)
{
	initializeSockets();
    std::ostringstream oss;
	oss << "Server started successfully" << std::endl;
    _logger->info(oss.str());
}

/**
 * Run one iteration of the server event loop
 */
void	Server::run(void)
{
    // Create temporary copies for select() to modify
    fd_set readFdsCopy;
    fd_set writeFdsCopy;
    fd_set errorFdsCopy;
    
    // Copy the master sets to the temporary sets
    readFdsCopy = _readFds;
    writeFdsCopy = _writeFds;
    errorFdsCopy = _errorFds;
    
    // Show which file descriptors we're monitoring
    int actualFdCount = 0;
    for (int i = 0; i <= _maxFd; i++) {
        if (FD_ISSET(i, &_readFds) || FD_ISSET(i, &_writeFds)) {
            std::ostringstream oss;
            oss << "DEBUG: Monitoring fd " << i << std::endl;
            _logger->debug(oss.str());
            actualFdCount++;
        }
    }
    std::ostringstream oss;
    oss << "DEBUG: select() checking range 0-" << _maxFd 
    << " (" << _maxFd + 1 << " total), actually monitoring " 
    << actualFdCount << " file descriptors" << std::endl;
    _logger->debug(oss.str());
    
    int activity = select(_maxFd + 1, &readFdsCopy, &writeFdsCopy, 
        &errorFdsCopy, NULL);
    
    if (activity < 0)
    {
        if (errno != EINTR) // Ignore if interrupted by signal
        std::ostringstream oss;
        oss << "Select error: " << strerror(errno) << std::endl;
        _logger->error(oss.str());
        return;
    }
    else if (activity == 0) 
    {
        // Timeout occurred, no activity
        return;
    }
    std::ostringstream oss1;
    oss1 << "DEBUG: select() returned " << activity 
    << " ready file descriptors" << std::endl;
    _logger->debug(oss.str());
      
    // Process I/O events
    acceptConnections(&readFdsCopy);
    handleRequests(&readFdsCopy);
    sendResponses(&writeFdsCopy);
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
	
    std::ostringstream oss;
	oss << "Server stopped" << std::endl;
    _logger->info(oss.str());
}
