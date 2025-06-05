#!/bin/bash

# WebServ Enhancement Test Suite
# Tests GET, POST, DELETE methods with static files and uploads

set -e

HOST="localhost"
PORT="8080"
BASE_URL="http://${HOST}:${PORT}"
TEST_DIR="/tmp/webserv_tests"
UPLOAD_FILE="${TEST_DIR}/test_upload.txt"
LARGE_FILE="${TEST_DIR}/large_test.bin"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Test counters
TESTS_RUN=0
TESTS_PASSED=0
TESTS_FAILED=0

# Helper functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[PASS]${NC} $1"
    ((TESTS_PASSED++))
}

log_error() {
    echo -e "${RED}[FAIL]${NC} $1"
    ((TESTS_FAILED++))
}

log_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

run_test() {
    ((TESTS_RUN++))
    echo -e "\n${BLUE}==================== TEST $TESTS_RUN ====================${NC}"
    echo -e "${BLUE}$1${NC}"
}

setup_test_env() {
    log_info "Setting up test environment..."
    mkdir -p "$TEST_DIR"
    
    # Create test upload file
    echo "This is a test file for upload testing.
It contains multiple lines.
Created at: $(date)
Test data: 1234567890" > "$UPLOAD_FILE"
    
    # Create a larger file for upload testing
    dd if=/dev/urandom of="$LARGE_FILE" bs=1024 count=100 2>/dev/null
    
    log_info "Test files created in $TEST_DIR"
}

cleanup_test_env() {
    log_info "Cleaning up test environment..."
    rm -rf "$TEST_DIR"
}

check_server_running() {
    log_info "Checking if WebServ is running on $BASE_URL..."
    if curl -s --connect-timeout 5 "$BASE_URL" > /dev/null 2>&1; then
        log_success "Server is running"
        return 0
    else
        log_error "Server is not running. Please start WebServ first."
        log_info "To start: ./webserv conf/default.conf"
        exit 1
    fi
}

# TEST 1: GET Method - Static File Serving
test_get_static_files() {
    run_test "GET Method - Static File Serving"
    
    # Test index.html
    log_info "Testing GET /"
    response=$(curl -s -w "%{http_code}" "$BASE_URL/")
    http_code="${response: -3}"
    if [ "$http_code" = "200" ]; then
        log_success "GET / returned 200 OK"
    else
        log_error "GET / returned $http_code (expected 200)"
    fi
    
    # Test specific HTML file
    log_info "Testing GET /upload.html"
    response=$(curl -s -w "%{http_code}" "$BASE_URL/upload.html")
    http_code="${response: -3}"
    if [ "$http_code" = "200" ]; then
        log_success "GET /upload.html returned 200 OK"
    else
        log_error "GET /upload.html returned $http_code (expected 200)"
    fi
    
    # Test text file
    log_info "Testing GET /test.txt"
    response=$(curl -s -w "%{http_code}" "$BASE_URL/test.txt")
    http_code="${response: -3}"
    if [ "$http_code" = "200" ]; then
        log_success "GET /test.txt returned 200 OK"
    else
        log_error "GET /test.txt returned $http_code (expected 200)"
    fi
    
    # Test 404 for non-existent file
    log_info "Testing GET /nonexistent.html (404 test)"
    response=$(curl -s -w "%{http_code}" "$BASE_URL/nonexistent.html")
    http_code="${response: -3}"
    if [ "$http_code" = "404" ]; then
        log_success "GET /nonexistent.html returned 404 Not Found"
    else
        log_error "GET /nonexistent.html returned $http_code (expected 404)"
    fi
}

# TEST 2: MIME Type Detection
test_mime_types() {
    run_test "MIME Type Detection"
    
    # Test HTML MIME type
    log_info "Testing HTML MIME type"
    content_type=$(curl -s -I "$BASE_URL/upload.html" | grep -i "content-type" | cut -d' ' -f2- | tr -d '\r\n')
    if [[ "$content_type" == *"text/html"* ]]; then
        log_success "HTML file has correct MIME type: $content_type"
    else
        log_error "HTML file has incorrect MIME type: $content_type"
    fi
    
    # Test CSS MIME type
    log_info "Testing CSS MIME type"
    content_type=$(curl -s -I "$BASE_URL/style.css" | grep -i "content-type" | cut -d' ' -f2- | tr -d '\r\n')
    if [[ "$content_type" == *"text/css"* ]]; then
        log_success "CSS file has correct MIME type: $content_type"
    else
        log_error "CSS file has incorrect MIME type: $content_type"
    fi
    
    # Test text MIME type
    log_info "Testing text MIME type"
    content_type=$(curl -s -I "$BASE_URL/test.txt" | grep -i "content-type" | cut -d' ' -f2- | tr -d '\r\n')
    if [[ "$content_type" == *"text/plain"* ]]; then
        log_success "Text file has correct MIME type: $content_type"
    else
        log_error "Text file has incorrect MIME type: $content_type"
    fi
}

# TEST 3: POST Method - Simple File Upload
test_post_simple_upload() {
    run_test "POST Method - Simple File Upload"
    
    log_info "Testing simple POST file upload"
    response=$(curl -s -w "%{http_code}" -X POST \
        -H "Content-Type: application/octet-stream" \
        --data-binary "@$UPLOAD_FILE" \
        "$BASE_URL/")
    
    http_code="${response: -3}"
    if [ "$http_code" = "201" ]; then
        log_success "Simple file upload returned 201 Created"
    else
        log_error "Simple file upload returned $http_code (expected 201)"
    fi
}

# TEST 4: POST Method - Multipart Form Upload
test_post_multipart_upload() {
    run_test "POST Method - Multipart Form Upload"
    
    log_info "Testing multipart/form-data file upload"
    response=$(curl -s -w "%{http_code}" \
        -F "file=@$UPLOAD_FILE" \
        "$BASE_URL/")
    
    http_code="${response: -3}"
    if [ "$http_code" = "201" ]; then
        log_success "Multipart file upload returned 201 Created"
    else
        log_error "Multipart file upload returned $http_code (expected 201)"
    fi
}

# TEST 5: POST Method - Form Data
test_post_form_data() {
    run_test "POST Method - Form Data"
    
    log_info "Testing form data submission"
    response=$(curl -s -w "%{http_code}" \
        -X POST \
        -H "Content-Type: application/x-www-form-urlencoded" \
        -d "name=test&message=hello+world" \
        "$BASE_URL/")
    
    http_code="${response: -3}"
    if [ "$http_code" = "200" ]; then
        log_success "Form data submission returned 200 OK"
    else
        log_error "Form data submission returned $http_code (expected 200)"
    fi
}

# TEST 6: DELETE Method
test_delete_method() {
    run_test "DELETE Method - File Deletion"
    
    # First create a file to delete
    log_info "Creating test file for deletion"
    echo "This file will be deleted" > "/workspaces/webserv/www/delete_me.txt"
    
    # Verify file exists
    log_info "Verifying test file exists"
    response=$(curl -s -w "%{http_code}" "$BASE_URL/delete_me.txt")
    http_code="${response: -3}"
    if [ "$http_code" = "200" ]; then
        log_success "Test file exists and is accessible"
    else
        log_error "Test file is not accessible (code: $http_code)"
        return
    fi
    
    # Delete the file
    log_info "Testing DELETE /delete_me.txt"
    response=$(curl -s -w "%{http_code}" -X DELETE "$BASE_URL/delete_me.txt")
    http_code="${response: -3}"
    if [ "$http_code" = "204" ]; then
        log_success "DELETE returned 204 No Content"
    else
        log_error "DELETE returned $http_code (expected 204)"
    fi
    
    # Verify file is gone
    log_info "Verifying file was deleted"
    response=$(curl -s -w "%{http_code}" "$BASE_URL/delete_me.txt")
    http_code="${response: -3}"
    if [ "$http_code" = "404" ]; then
        log_success "File was successfully deleted (404 Not Found)"
    else
        log_error "File still exists after deletion (code: $http_code)"
    fi
    
    # Test DELETE on non-existent file
    log_info "Testing DELETE on non-existent file"
    response=$(curl -s -w "%{http_code}" -X DELETE "$BASE_URL/does_not_exist.txt")
    http_code="${response: -3}"
    if [ "$http_code" = "404" ]; then
        log_success "DELETE on non-existent file returned 404"
    else
        log_error "DELETE on non-existent file returned $http_code (expected 404)"
    fi
}

# TEST 7: Path Traversal Security
test_path_traversal() {
    run_test "Path Traversal Security"
    
    # Test various path traversal attempts
    declare -a traversal_paths=(
        "/../etc/passwd"
        "/../../etc/passwd"
        "/../../../etc/passwd"
        "/..%2F..%2F..%2Fetc%2Fpasswd"
        "/....//....//....//etc/passwd"
    )
    
    for path in "${traversal_paths[@]}"; do
        log_info "Testing path traversal: $path"
        response=$(curl -s -w "%{http_code}" "$BASE_URL$path")
        http_code="${response: -3}"
        if [ "$http_code" = "403" ] || [ "$http_code" = "404" ]; then
            log_success "Path traversal blocked: $path (code: $http_code)"
        else
            log_error "Path traversal not blocked: $path (code: $http_code)"
        fi
    done
}

# TEST 8: Directory Listing (Autoindex)
test_directory_listing() {
    run_test "Directory Listing (Autoindex)"
    
    # Create a test subdirectory
    mkdir -p "/workspaces/webserv/www/testdir"
    echo "File in subdirectory" > "/workspaces/webserv/www/testdir/subfile.txt"
    
    log_info "Testing directory listing for /testdir/"
    response=$(curl -s -w "%{http_code}" "$BASE_URL/testdir/")
    http_code="${response: -3}"
    if [ "$http_code" = "200" ]; then
        log_success "Directory listing returned 200 OK"
        
        # Check if the response contains directory listing
        content=$(curl -s "$BASE_URL/testdir/")
        if [[ "$content" == *"Index of"* ]] && [[ "$content" == *"subfile.txt"* ]]; then
            log_success "Directory listing contains expected content"
        else
            log_error "Directory listing does not contain expected content"
        fi
    else
        log_error "Directory listing returned $http_code (expected 200)"
    fi
    
    # Cleanup
    rm -rf "/workspaces/webserv/www/testdir"
}

# TEST 9: Large File Upload
test_large_file_upload() {
    run_test "Large File Upload"
    
    log_info "Testing large file upload (100KB)"
    response=$(curl -s -w "%{http_code}" -X POST \
        -H "Content-Type: application/octet-stream" \
        --data-binary "@$LARGE_FILE" \
        "$BASE_URL/")
    
    http_code="${response: -3}"
    if [ "$http_code" = "201" ]; then
        log_success "Large file upload returned 201 Created"
    else
        log_error "Large file upload returned $http_code (expected 201)"
    fi
}

# TEST 10: Method Not Allowed
test_method_not_allowed() {
    run_test "Method Not Allowed"
    
    log_info "Testing PATCH method (should be not allowed)"
    response=$(curl -s -w "%{http_code}" -X PATCH "$BASE_URL/")
    http_code="${response: -3}"
    if [ "$http_code" = "501" ] || [ "$http_code" = "405" ]; then
        log_success "PATCH method correctly rejected (code: $http_code)"
    else
        log_error "PATCH method returned $http_code (expected 501 or 405)"
    fi
}

print_summary() {
    echo -e "\n${BLUE}==================== TEST SUMMARY ====================${NC}"
    echo -e "Tests Run:    ${BLUE}$TESTS_RUN${NC}"
    echo -e "Tests Passed: ${GREEN}$TESTS_PASSED${NC}"
    echo -e "Tests Failed: ${RED}$TESTS_FAILED${NC}"
    
    if [ $TESTS_FAILED -eq 0 ]; then
        echo -e "\n${GREEN}🎉 ALL TESTS PASSED! 🎉${NC}"
        exit 0
    else
        echo -e "\n${RED}❌ SOME TESTS FAILED ❌${NC}"
        exit 1
    fi
}

# Main execution
main() {
    echo -e "${BLUE}WebServ Enhancement Test Suite${NC}"
    echo -e "${BLUE}Testing GET, POST, DELETE methods with static files and uploads${NC}\n"
    
    setup_test_env
    check_server_running
    
    # Run all tests
    test_get_static_files
    test_mime_types
    test_post_simple_upload
    test_post_multipart_upload
    test_post_form_data
    test_delete_method
    test_path_traversal
    test_directory_listing
    test_large_file_upload
    test_method_not_allowed
    
    cleanup_test_env
    print_summary
}

# Run main function
main "$@"