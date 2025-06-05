# WebServ Test Suite

This directory contains comprehensive tests for the WebServ HTTP server enhancements, validating GET, POST, and DELETE methods with static file serving and upload functionality.

## Test Structure

### 🧪 Unit Tests (`test_unit.cpp`)
- **MIME Type Detection**: Tests proper content-type header generation for various file extensions
- **Path Safety**: Validates security measures against path traversal attacks
- **Request Parsing**: Basic HTTP request parsing functionality

### 🌐 Integration Tests (`test_webserv.sh`)
- **GET Method**: Static file serving, directory listing, 404 handling
- **POST Method**: File uploads (simple, multipart, form data)
- **DELETE Method**: File deletion with proper status codes
- **Security**: Path traversal protection tests
- **MIME Types**: Content-type validation for different file types
- **Error Handling**: 404, 403, 405, 501 status codes

### ⚡ Performance Tests (`test_performance.sh`)
- **Concurrent Requests**: Multiple simultaneous connections
- **Sequential Load**: High-volume request testing
- **File Type Performance**: Different content types
- **Large File Handling**: 1MB+ file serving
- **Error Performance**: 404 response times

## Running Tests

### Quick Test (All tests)
```bash
./tests/run_tests.sh
```

### Unit Tests Only
```bash
./tests/run_tests.sh --unit-only
```

### Integration Tests Only
```bash
./tests/run_tests.sh --integration-only
```

### Performance Tests
```bash
# Start WebServ first
./webserv conf/default.conf &

# Run performance tests
./tests/test_performance.sh
```

### Manual Integration Tests
```bash
# Start WebServ first
./webserv conf/default.conf &

# Run integration tests
./tests/test_webserv.sh
```

## Test Options

### run_tests.sh Options
- `-u, --unit-only`: Run only unit tests
- `-i, --integration-only`: Run only integration tests  
- `-s, --skip-build`: Skip building (use existing binaries)
- `-l, --show-logs`: Show server logs after tests
- `-h, --help`: Show help message

## Prerequisites

### Required Tools
- `curl` - For HTTP requests
- `bc` - For performance calculations
- `make` - For building
- `c++` - For compilation

### Server Setup
1. Build WebServ: `make`
2. Create uploads directory: `mkdir uploads`
3. Start server: `./webserv conf/default.conf`

## Test Files Created

The test suite creates several test files:
- `/www/test.txt` - Plain text file
- `/www/style.css` - CSS file for MIME testing
- `/www/upload.html` - File upload interface
- `/uploads/` - Directory for file uploads

## Expected Results

### Unit Tests
- ✅ All MIME type mappings correct
- ✅ Path traversal attempts blocked
- ✅ Request parsing works properly

### Integration Tests
- ✅ GET: 200 for existing files, 404 for missing
- ✅ POST: 201 for uploads, 200 for form data
- ✅ DELETE: 204 for successful deletion, 404 for missing files
- ✅ Security: 403/404 for path traversal attempts
- ✅ MIME: Correct content-type headers

### Performance Tests
- ✅ Server handles concurrent requests
- ✅ Response times under reasonable limits
- ✅ No memory leaks or crashes under load

## Troubleshooting

### Server Not Starting
- Check if port 8080 is available
- Verify configuration file exists
- Check for compilation errors

### Tests Failing
- Ensure server is running: `curl http://localhost:8080`
- Check server logs: `./webserv conf/default.conf`
- Verify file permissions in `www/` and `uploads/`

### Common Issues
1. **Port in use**: Change port in `conf/default.conf`
2. **Permission denied**: Check directory permissions
3. **404 errors**: Verify test files exist in `www/`
4. **Upload failures**: Ensure `uploads/` directory exists

## Test Coverage

The test suite validates:
- ✅ HTTP/1.1 compliance
- ✅ Static file serving
- ✅ File upload functionality
- ✅ CRUD operations (Create, Read, Delete)
- ✅ Security measures
- ✅ Error handling
- ✅ MIME type detection
- ✅ Performance under load
- ✅ Concurrent connection handling

## Continuous Integration

For CI/CD integration:
```bash
# Full test suite with exit codes
./tests/run_tests.sh
echo "Exit code: $?"
```

The test suite returns:
- `0` - All tests passed
- `1` - Some tests failed