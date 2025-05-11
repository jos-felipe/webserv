/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: josfelip <josfelip@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/26 12:55:47 by josfelip          #+#    #+#             */
/*   Updated: 2025/04/28 16:06:28 by josfelip         ###   ########.fr       */
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
  _epollFd = epoll_create(0);
  if (_epollFd == -1) {
    throw std::runtime_error("Failed to create epoll instance: " + std::string(strerror(errno)));
  }
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
            std::cout << "DEBUG: Listen socket " << listenFd 
                << " is ready for accepting" << std::endl;
                
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
        std::cout << "DEBUG: Client socket " << clientFd 
                << " is ready for reading" << std::endl;
                
      try
      {
        if (!_requests.count(clientFd))
        {
            std::cout << "DEBUG: Creating new request for fd " 
                        << clientFd << std::endl;
          _requests[clientFd] = HttpRequest();
        }
          
        HttpRequest& request = _requests[clientFd];
        
        bool requestComplete = request.read(it->second);
        std::cout << "DEBUG: Request read returned: " 
                    << (requestComplete ? "COMPLETE" : "INCOMPLETE") << std::endl;
        
        if (requestComplete)
        {
          // Request is complete, process it
          std::cout << "DEBUG: Processing request and generating response" 
                        << std::endl;
          HttpResponse response = request.process(*_config);
          _responses[clientFd] = response;
          
          // Switch to writing mode
          std::cout << "DEBUG: Switching socket " << clientFd 
                        << " to write mode" << std::endl;
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
void Server::sendResponses(void)
{
    std::cout << "DEBUG: Checking for sockets ready to write" << std::endl;
    
    std::vector<int> toRemove;
    std::vector<int> toKeepAlive;  // New vector to track connections to keep alive
    
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
        
        std::cout << "DEBUG: Checking if socket " << clientFd 
            << " is ready for writing: " 
            << (FD_ISSET(clientFd, &_writeFds) ? "YES" : "NO") << std::endl;
            
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
                
                std::cout << "DEBUG: Attempting to send response on fd " 
                    << clientFd << std::endl;
                    
                if (response.send(_clientSockets[clientFd]))
                {
                    // Response fully sent, either keep-alive or close
                    std::cout << "DEBUG: Response fully sent on fd " 
                        << clientFd << std::endl;
                        
                    if (response.shouldKeepAlive())
                    {
                        // Mark for keep-alive processing AFTER we finish iterating
                        std::cout << "DEBUG: Marking connection for keep-alive: " 
                            << clientFd << std::endl;
                        toKeepAlive.push_back(clientFd);
                    }
                    else
                    {
                        std::cout << "DEBUG: Connection will be closed" << std::endl;
                        toRemove.push_back(clientFd);
                    }
                }
                else
                {
                    std::cout << "DEBUG: Response not fully sent yet, "
                        << "will try again later" << std::endl;
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
        std::cout << "DEBUG: Keeping connection alive, "
            << "switching " << clientFd << " back to read mode" << std::endl;
        
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
        std::cout << "DEBUG: Closing connection: " << clientFd << std::endl;
        
        FD_CLR(clientFd, &_readFds);
        FD_CLR(clientFd, &_writeFds);
        _clientSockets.erase(clientFd);
        _requests.erase(clientFd);
        _responses.erase(clientFd);
        close(clientFd);
        std::cout << "Connection closed: fd " << clientFd << std::endl;
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

void	Server::registerFd(int fd, uint32_t events) {
  struct epoll_event	ev;
  ev.events = events;
  ev.data.fd = fd;

  if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &ev) == -1) {
    throw std::runtime_error("Failed to add fd to epoll: " + std::string(strerror(errno)));
  }
}

void	Server::modifyFd(int fd, uint32_t events) {
  struct epoll_event	ev;
  ev.events = events;
  ev.data.fd = fd;

  if (epoll_ctl(_epollFd, EPOLL_CTL_MOD, fd, &ev) == -1) {
    throw std::runtime_error("Failed to modify fd in epoll: " + std::string(strerror(errno)));
  }
}

void	Server::unregisterFd(int fd) {
  if (epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, NULL) == -1)
  {
    std::cerr << "Failed to remove fd from epoll: " << strerror(errno) << std::endl;
  }
}

void	Server::setReadable(int fd, bool enable) {
  struct epoll_event	ev;
  ev.data.fd = fd;

  bool	isClientSocket = (_clientSockets.find(fd) != _clientSockets.end());
  
  bool	isListenSocket = (
    std::find(
      _listenSockets.begin(),
      _listenSockets.end(),
      Socket(fd, sockaddr_in())
    )
    !=
    _listenSockets.end()
  );
  
  if (!isClientSocket && !isListenSocket) {
    std::cerr << "Socket not found for fd " << fd << std::endl;
  }
  
  if (enable)
    ev.events = EPOLLIN;
  else
    ev.events = 0;

  if (_responses.find(fd) != _responses.end())
    ev.events |= EPOLLOUT;
  
  if (epoll_ctl(_epollFd, EPOLL_CTL_MOD, fd, &ev) == -1) {
    if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &ev) == -1) {
      std::cerr << "Failed to set up epoll events for fd" << fd << std::endl;
    }
  }
}

/**
 * Set a file descriptor as writable in the epoll instance
 * 
 * This method configures a socket to be monitored for write readiness.
 * When enabled, epoll will notify us when the socket can accept data.
 */
void	Server::setWritable(int fd, bool enable) {
  bool	isClientSocket = (_clientSockets.find(fd) != _clientSockets.end());

  if (!isClientSocket) {
    std::cerr << "Client socket not found for fd " << fd << std::endl;
    return;
  }

  struct epoll_event	ev;
  ev.data.fd = fd;

  if (enable) {
    ev.events = EPOLLOUT;
    
  } else {
    ev.events = 0;
  }

  // full-duplex channel concern
  if (_responses.find(fd) == _responses.end()) {
    ev.events |= EPOLLIN;
  }

  if (epoll_ctl(_epollFd, EPOLL_CTL_MOD, fd, &ev) == -1) {
    if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &ev) == -1) {
      std::cerr << "Failed to set up epoll write evets for fd " << fd << std::endl;
    }
  }
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
 * Run one iteration of the server event loop using epoll
 */
void	Server::run(void)
{
  const int	MAX_EVENTS = 64;
  const int	TIMEOUT = 1000; // in ms (1s)
  struct epoll_event	events[MAX_EVENTS];
  
  int	numEvents = epoll_wait(_epollFd, events, MAX_EVENTS, TIMEOUT);

  if (numEvents == -1) {
    std::cerr << "epoll_wait error occurred" << std::endl;
    return;
  }
  
  if (numEvents)
    return;
    
  std::cout << "DEBUG: epoll_wait returned " << numEvents << " events" << std::endl;

  processEvents(events, numEvents);
  
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

void	Server::processEvents(struct epoll_event *events, int numEvents) {
  for (int i = 0; i < numEvents; i++) {
    int			fd = events[i].data.fd;
    uint32_t	eventType = events[i].events;
    
    bool	isListenSocket = false;
    for (std::vector<Socket>::const_iterator it = _listenSockets.begin(); it != _listenSockets.end(); ++it) {
      if (it->getFd() == fd) {
        isListenSocket = true;
        break;
      }
    }
    
    if (isListenSocket) {
      handleNewConnection(fd);
    }
    else if (eventType & EPOLLIN) {
      handleClientRead(fd);
    }
    else if (eventType & EPOLLOUT) {
      handleClientWrite(fd);
    }
    else if (eventType & (EPOLLERR | EPOLLHUP)) {
      handleClientError(fd);
    }	
  }
}

void	Server::handleNewConnection(int listenFd) {
  try {
    Socket* listenSocket = NULL;
    for (std::vector<Socket>::iterator it = _listenSockets.begin(); it != _listenSockets.end(); ++it) {
      if (it->getFd() == listenFd) {
        listenSocket = &(*it);
        break ;
      }
    }
    
    if (!listenSocket) {
      std::cerr << "Error: Listening socket not found for fd " << listenFd << std::endl;
      return ;
    }
    Socket clientSocket = listenSocket->accept();
  
    if (clientSocket.getFd() >= 0) {
      clientSocket.setNonBlocking();
  
      int clientFd = clientSocket.getFd();
      _clientSockets[clientFd] = clientSocket;
  
      setReadble(clientFd, true);
      
      std::cout << "New connection accepted: fd " << clientFd << std::endl;
    }
  }
  catch (const std::exception& e) {
    std::cerr << "Failed to accept connection: " << e.what() << std::endl;
  }
}

void Server::handleCientRead(int ClientFd) {
  if (!_config)
    return ;

  try {
    if (_clientSockets.find(clientFd) == _clientSockets.end()) {
      std::cerr << "Error: Client socket not found for fd " << clientFd << std::endl;
      return ;
    }

    std::cout << "DEBUG: Client socket " << clientFd << " is ready for reading" << std::endl;

    if (!_requests.count(clientFd)) {
      std::cout << "DEBUG: Creating new request for fd " << clientFd << std::endl;
      _requests[clientFd] = HttpRequest();
    }

    HttpRequest& request = _requests[clientFd];

    bool requestComplete = request.eead(_clientSockets[clientFd]);
    std::cout <<"DEBUG: Request read returned: " << (requestComplete ? "COMPLETE" : "INCOMPLETE") << std::endl;
    
    if (requestComplete) {
      std::cout << "DEBUG: Processing request and generating response" << std::endl;
      HttpResponse response = request.process(*_config);
      _responses[clientFd] = response;

      std::cout << "DEBUG: Switching socket " << clientFd << " to write mode" << std::endl;
      setWritable(clientFd, true);
    }
  }
  catch (const std::exception& e) {
    std::cerr << "Error handling request on fd " << clientFd << ": " << e.what() << std::endl;
    handleClientError(clientFd);
  }
}

void Server::handleClientWrite(int clientFd) {
  try {
    if (_responses.find(clientFd) == _responses.end()) {
      std::cerr << "No response queued for fd " << clientFd << std::endl;
      return ;
    }

    HttpResponse& response = _responses[clientFd];

    if (_clientSockets.find(clientFd) == _clientSockets.end())
    {
      std::cerr << "Client socket not found for fd " << clientFd << std::endl;
      return ;
    }

    std::cout << "DEBUG: Attempting to send response on fd " << clientFd << std::endl;

    if (response.send(_clientSockets[clientFd])) {
      std::cout << "DEBUG: Response fully sent on fd " << clientFd << std::endl;
      
      if (response.shouldKeepAlive()) {
        std::cout << "DEBUG: keeping connection alive, switching " << clientFd << " back to read mode" << std::endl;
  
        _requests.erase(clientFd);
        _responses.erase(clientFd);
        setReadable(clientFd, true);
      } else {
        std::cout << "DEBUG: Connection will be closed" << std::endl;
        handleClientError(clientFd);
      }
    }
  }
  catch (const std::exception& e) {
    std::cerr << "Error sending response on fd " << clientFd << ": " << e.what() << std::endl;
    handleClientError(clientFd);
  }
}