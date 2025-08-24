# Virtual Hosting Test Setup

This document explains the virtual hosting test infrastructure created for WebServ.

## Overview

The setup tests WebServ's ability to serve different content based on hostname, validating the `Config::findServer()` method's virtual hosting functionality.

## Directory Structure

```
www-vhosts/
├── naruto.com/           # Virtual host for naruto.com:3000
│   ├── index.html        # Homepage with orange theme
│   ├── about.html        # About page with technical details
│   └── style.css         # Custom CSS styling
├── sasuke.com/           # Virtual host for sasuke.com:3000  
│   ├── index.html        # Homepage with dark blue theme
│   ├── team.html         # Team page with validation info
│   └── style.css         # Custom CSS styling
└── localhost/            # Default virtual host for localhost:8080
    ├── index.html        # Default homepage
    ├── admin.html        # Administrative page
    └── style.css         # Standard CSS styling
```

## Configuration (conf/eval.conf)

Three server blocks configured:

1. **localhost:8080** - Default host (`./www-vhosts/localhost`)
2. **naruto.com:3000** - First virtual host (`./www-vhosts/naruto.com`) 
3. **sasuke.com:3000** - Second virtual host (`./www-vhosts/sasuke.com`)

## Test Scripts

### test_diff_hostnames.sh
Enhanced basic test script with:
- ✅ 9 comprehensive tests
- ✅ Color-coded output
- ✅ Content validation
- ✅ Pass/fail reporting
- ✅ CSS file serving tests

### test_virtual_hosting_comprehensive.sh
Advanced test suite with:
- ✅ 8 test sections
- ✅ HTTP header validation
- ✅ Performance testing
- ✅ Error handling
- ✅ Content differentiation
- ✅ Multiple HTTP methods
- ✅ Detailed reporting

## Running the Tests

### Prerequisites
1. Build WebServ: `make`
2. Start server: `./webserv conf/eval.conf`

### Basic Testing
```bash
./test_diff_hostnames.sh
```

### Comprehensive Testing  
```bash
./test_virtual_hosting_comprehensive.sh
```

## Manual Testing Commands

### Test naruto.com
```bash
curl --resolve naruto.com:3000:127.0.0.1 http://naruto.com:3000/
curl --resolve naruto.com:3000:127.0.0.1 http://naruto.com:3000/about.html
curl --resolve naruto.com:3000:127.0.0.1 http://naruto.com:3000/style.css
```

### Test sasuke.com
```bash
curl --resolve sasuke.com:3000:127.0.0.1 http://sasuke.com:3000/
curl --resolve sasuke.com:3000:127.0.0.1 http://sasuke.com:3000/team.html
curl --resolve sasuke.com:3000:127.0.0.1 http://sasuke.com:3000/style.css
```

### Test localhost (default)
```bash
curl http://localhost:8080/
curl http://localhost:8080/admin.html
curl http://localhost:8080/style.css
```

## What This Tests

### Core Functionality
- **Hostname-based routing** - Different content per domain
- **Port-based separation** - Multiple hosts on same port
- **Document root isolation** - Separate content trees
- **Server name matching** - `Config::findServer()` method
- **Default host fallback** - Localhost as fallback

### Technical Validation
- **HTTP/1.1 compliance** - Proper Host header handling
- **Static file serving** - CSS, HTML files per virtual host
- **Error handling** - 404 pages per virtual host
- **Performance** - Response time consistency
- **Content isolation** - No cross-contamination between hosts

## Expected Results

When virtual hosting works correctly:
1. Each hostname serves unique content
2. CSS styles are different per virtual host  
3. All tests pass with 100% success rate
4. No content mixing between virtual hosts
5. Default host serves fallback content

## Troubleshooting

### Common Issues
- **Same content served**: Check document root paths in config
- **Connection refused**: Ensure WebServ is running with eval.conf
- **404 errors**: Verify file paths and permissions
- **DNS resolution**: Use `--resolve` flag with curl

### Debug Commands
```bash
# Check server status
curl -I http://localhost:8080/

# Verify configuration parsing
./webserv conf/eval.conf --debug

# Test DNS resolution bypass
curl -v --resolve naruto.com:3000:127.0.0.1 http://naruto.com:3000/
```

This setup comprehensively validates WebServ's virtual hosting implementation as required by the HTTP/1.1 specification.