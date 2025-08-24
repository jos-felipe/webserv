# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Build the server
make

# Build with debug information and logging
make debug

# Run the server with default config
make run
# or specify config file
./webserv [config_file]

# Clean object files
make clean

# Full clean (removes executable and debugme)
make fclean

# Rebuild from scratch
make re
```

The debug build creates `debugme` binary with `-g3` flags and `LOG_LEVEL=LOG_DEBUG` macro for verbose logging.

## Core Architecture

WebServ is a C++98 HTTP/1.1 server implementing **non-blocking I/O** as a core requirement. The architecture follows a modular design with strict separation between network handling, HTTP processing, and content serving.

### Main Event Loop (`Server.cpp`)
- Uses `select()`/`poll()`/`epoll()`/`kqueue()` for I/O multiplexing
- **Critical**: ALL socket operations must go through the multiplexer - direct `read()`/`write()` calls will result in grade 0
- Manages multiple listening sockets and client connections simultaneously
- Handles connection lifecycle and cleanup on disconnection

### Configuration System (`Config.cpp`)
The server uses NGINX-inspired configuration with two-level hierarchy:

**Server Blocks**: Define virtual servers by host:port binding
**Location Blocks**: Define route-specific behavior with longest-prefix matching

Key parsing logic:
- `parseServerBlock()`: Handles server-level directives (listen, server_name, error_page)
- `parseLocationBlock()`: Handles route-level directives (method, root, return, cgi_ext)
- `findServer()`: Matches incoming requests by host:port + server_name with fallback logic

### HTTP Request Processing (`HttpRequest.cpp`)
Stateful parser implementing the complete HTTP/1.1 parsing pipeline:

**ParseState enum**: REQUEST_LINE → HEADERS → BODY → COMPLETE
- Handles chunked encoding (`parseChunkedSize()`, `parseChunkedData()`)
- Supports partial reads in non-blocking mode

**Route Resolution Flow**:
1. `findLocation()` - longest-prefix matching algorithm
2. **Redirect check** (highest priority) - processes `return` directives immediately
3. Method validation against allowed methods per location
4. Content processing based on request type

### CGI Execution (`CgiHandler.cpp`)
Implements CGI/1.1 specification for dynamic content:
- Uses `fork()`/`execve()` pattern (only allowed use of `fork()` in the project)
- Sets up proper environment variables (PATH_INFO, QUERY_STRING, etc.)
- Pipe-based communication with timeout handling
- Supports .php, .py, and other configured extensions

### Socket Abstraction (`Socket.cpp`)
Reference-counted wrapper preventing premature socket closure:
- Shared socket instances across connections
- Non-blocking mode setup with `fcntl()` (MacOS compatibility)
- Proper cleanup on reference count reaching zero

## Request Processing Flow

1. **Server Selection**: `Config::findServer()` matches by host:port + server_name
2. **Location Matching**: `HttpRequest::findLocation()` uses longest-prefix algorithm
3. **Redirect Processing**: Check `location.redirect` field **before** any content handling
4. **Method Validation**: Verify HTTP method allowed for location
5. **Content Processing**: Route to appropriate handler (static, CGI, upload)

### Critical Design Constraints

**C++98 Compliance**: No STL beyond basic containers, no external libraries, standard library only

**Non-blocking I/O Requirement**: 
- Must use single `poll()` (or equivalent) for all I/O operations
- Never call `read()`/`write()` without checking file descriptor readiness
- Checking `errno` after read/write operations is forbidden

**Allowed System Calls**: `execve`, `pipe`, `fork`, `socketpair`, network functions (`socket`, `bind`, `listen`, `accept`, `send`, `recv`), I/O multiplexing (`select`, `poll`, `epoll`, `kqueue`), file operations (`open`, `read`, `write`, `stat`)

## Configuration Format

NGINX-style syntax with server and location blocks:

```nginx
server {
    listen 8080;
    server_name localhost;
    error_page 404 /404.html;
    client_max_body_size 10M;
    
    location / {
        root ./www;
        method GET POST DELETE;
        index index.html;
        autoindex on;
        upload_store ./www/uploads;
        cgi_ext .php .py;
    }
    
    location /redirect {
        return 301 /new-path;
    }
}
```

**Key Directives**:
- `return`: Creates 301/302 redirects (processed before content serving)
- `method`: Restricts allowed HTTP methods per location
- `cgi_ext`: Enables CGI processing for specified extensions
- `upload_store`: Configures file upload destination

## Testing and Validation

The server must be compatible with standard web browsers and pass comparison tests against NGINX behavior. Use tools like `curl`, `telnet`, and browsers for testing HTTP compliance.

**Stress Testing**: Server must remain available under high concurrent load without hanging or crashing.

**CGI Testing**: Verify proper environment variable setup and script execution in correct working directory.