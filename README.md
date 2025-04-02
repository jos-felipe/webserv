# WebServ - HTTP Server Implementation

<p align="center">
  <img src="https://img.shields.io/badge/language-C++-blue.svg" alt="Language">
  <img src="https://img.shields.io/badge/standard-C++98-yellow.svg" alt="C++ Standard">
</p>

## 📖 Overview
WebServ is a lightweight HTTP server implementation in C++98. This project is developed to understand the internals of HTTP protocol and web servers, providing hands-on experience with network programming, concurrent connections handling, and HTTP protocol implementation.

> _"This is when you finally understand why a URL starts with HTTP"_

## ✨ Features
- **Fully compliant with HTTP/1.1** protocol specifications
- **Non-blocking I/O operations** using poll/select/epoll/kqueue
- **Multiple virtual servers** support with different configurations
- **Method support**: GET, POST, DELETE
- **Static file serving** with directory listings
- **CGI support** for dynamic content
- **File uploads** handling
- **Error pages** customization
- **Redirections** configuration
- **Client body size limit** configuration

## 🧰 Requirements
- C++ compiler with C++98 support
- Make build system
- UNIX/Linux environment (or MacOS)
- No external libraries (standard library only)

## 🏗️ Project Structure
```
webserv/
├── src/
│   ├── config/        # Configuration parsing
│   ├── http/          # HTTP protocol handling
│   ├── socket/        # Socket operations
│   ├── server/        # Main server implementation
│   ├── cgi/           # CGI implementation
│   ├── utils/         # Utility functions and classes
│   └── main.cpp       # Entry point
├── include/           # Header files
├── conf/              # Configuration examples
├── www/               # Web content examples
├── tests/             # Test scripts and files
├── Makefile           # Build script
└── README.md          # This file
```

## 🚀 Building and Running
```bash
# Clone the repository
git clone https://github.com/jos-felipe/webserv.git
cd webserv

# Build the project
make

# Run with default configuration
./webserv

# Run with custom configuration
./webserv conf/custom.conf
```

## ⚙️ Configuration File
The server uses a configuration file inspired by NGINX to set up different server instances and routes. Here's a basic example:

```
# Simple server configuration
server {
    listen 8080;
    server_name example.com;
    root /var/www/html;
    
    # Default error pages
    error_page 404 /404.html;
    error_page 500 502 503 504 /50x.html;
    
    # Client body size limit (in bytes)
    client_max_body_size 10M;
    
    # Route configuration
    location / {
        # Accepted HTTP methods
        method GET POST;
        
        # Directory listing
        autoindex on;
        
        # Default file for directories
        index index.html;
    }
    
    # CGI configuration
    location ~ \.php$ {
        method GET POST;
        cgi_pass /usr/bin/php-cgi;
    }
    
    # Redirection example
    location /old {
        return 301 /new;
    }
    
    # Upload configuration
    location /upload {
        method POST;
        upload_store /tmp/uploads;
    }
}
```

## 📋 Development Progress
- [x] Project setup and repository initialization
- [x] Configuration file parsing
- [x] Socket management and non-blocking I/O
- [ ] HTTP request parsing
- [ ] HTTP response generation
- [ ] Static file serving
- [ ] Directory listing
- [ ] Error handling
- [ ] CGI implementation
- [ ] File upload handling
- [ ] Testing and documentation

## 🧪 Testing
The server can be tested using:
- Web browsers (Chrome, Firefox, Safari, etc.)
- Command-line tools (curl, wget)
- Testing scripts (Python, Bash)
- Telnet for raw HTTP request testing
- External tools like Apache Bench for stress testing

## 👥 Contributors
- [jos-felipe](https://github.com/jos-felipe)

## 📄 License
This project is part of the School 42 curriculum.