# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

- `make` - Build the main webserv binary with C++98 standard
- `make debug` - Build debug version (creates `debugme` binary with -g3 flags)
- `make clean` - Remove object files
- `make fclean` - Remove all build artifacts including binaries
- `make re` - Clean rebuild

## Testing Commands

- `./tests/run_tests.sh` - Run complete test suite (unit + integration)
- `./tests/run_tests.sh --unit-only` - Run only unit tests
- `./tests/run_tests.sh --integration-only` - Run only integration tests
- `./tests/test_performance.sh` - Run performance/load tests
- `./tests/test_webserv.sh` - Manual integration tests
- `./tests/test_cgi.sh` - CGI functionality tests

## Running the Server

- `./webserv` - Run with default configuration (conf/default.conf)
- `./webserv path/to/config.conf` - Run with custom configuration

## Architecture Overview

This is a C++98 HTTP/1.1 web server with nginx-inspired configuration. The main components interact as follows:

```
main.cpp → Config → Server → Socket/HttpRequest/HttpResponse
```

### Core Components

- **Server**: Main orchestrator using select() for non-blocking I/O multiplexing
- **Socket**: PIMPL pattern with reference counting for safe copying, handles both listening and client sockets
- **HttpRequest**: State machine parser for incremental HTTP/1.1 request parsing
- **HttpResponse**: Streaming response generator with keep-alive support
- **Config**: nginx-like configuration parser supporting virtual servers and location blocks
- **CgiHandler**: CGI script execution with fork/exec, environment setup, and output parsing

### Request Processing Flow

1. Server accepts connections via Socket
2. HttpRequest incrementally parses incoming data using ParseState enum
3. Requests are routed based on configuration (server blocks, location blocks)
4. If request is for CGI file (based on extension), CgiHandler executes script with fork/exec
5. Otherwise, HttpResponse generates appropriate responses (GET/POST/DELETE handlers)
6. Connections can be kept alive for reuse

## Configuration System

Uses nginx-inspired syntax with server blocks and location blocks:

```nginx
server {
    listen 8080;
    server_name localhost;
    error_page 404 /404.html;
    client_max_body_size 10M;
    
    location / {
        method GET POST DELETE;
        root ./www;
        index index.html;
        autoindex on;
        upload_store ./uploads;
    }
}
```

Key features: virtual servers, custom error pages, method restrictions, file uploads, CGI support, autoindex.

### CGI Configuration

- `cgi_ext .php .py .pl` - Specify file extensions that should be handled by CGI
- `cgi_pass /path/to/interpreter` - Optional: specify custom interpreter path
- CGI scripts must be executable and located within the configured root directory
- Default interpreters: .php → /usr/bin/php-cgi, .py → /usr/bin/python3, .pl → /usr/bin/perl

## Development Standards

- **C++98 compliance**: Strict adherence required
- **Non-blocking I/O**: All socket operations use select() multiplexing
- **RAII patterns**: Proper resource management, especially in Socket class
- **State machines**: Used for HTTP parsing and connection management
- **Path traversal protection**: Security validation throughout request handling
- **HTTP/1.1 compliance**: Full protocol support including keep-alive and chunked encoding
- **CGI/1.1 compliance**: Environment variables, PATH_INFO, fork/exec, pipe communication

## Testing Strategy

Three-tier approach: unit tests (basic parsing/security), integration tests (full HTTP functionality), and performance tests (concurrent connections). Tests automatically manage server lifecycle and include comprehensive security validation.