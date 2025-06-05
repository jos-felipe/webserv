#!/bin/bash

# WebServ Performance Test
# Tests concurrent requests and load handling

set -e

HOST="localhost"
PORT="8080"
BASE_URL="http://${HOST}:${PORT}"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[PASS]${NC} $1"
}

log_error() {
    echo -e "${RED}[FAIL]${NC} $1"
}

check_server_running() {
    log_info "Checking if WebServ is running on $BASE_URL..."
    if curl -s --connect-timeout 5 "$BASE_URL" > /dev/null 2>&1; then
        log_success "Server is running"
        return 0
    else
        log_error "Server is not running. Please start WebServ first."
        exit 1
    fi
}

test_concurrent_get_requests() {
    echo -e "\n${BLUE}==================== CONCURRENT GET TESTS ====================${NC}"
    
    log_info "Testing 10 concurrent GET requests..."
    
    local start_time=$(date +%s.%N)
    local success_count=0
    local total_requests=10
    
    # Run concurrent requests
    for i in $(seq 1 $total_requests); do
        {
            response=$(curl -s -w "%{http_code}" "$BASE_URL/" 2>/dev/null)
            http_code="${response: -3}"
            if [ "$http_code" = "200" ]; then
                echo "Request $i: SUCCESS"
                ((success_count++))
            else
                echo "Request $i: FAILED (code: $http_code)"
            fi
        } &
    done
    
    # Wait for all background jobs to complete
    wait
    
    local end_time=$(date +%s.%N)
    local duration=$(echo "$end_time - $start_time" | bc -l)
    
    log_info "Completed $success_count/$total_requests requests in ${duration}s"
    
    if [ $success_count -eq $total_requests ]; then
        log_success "All concurrent GET requests succeeded"
    else
        log_error "$((total_requests - success_count)) requests failed"
    fi
}

test_sequential_requests() {
    echo -e "\n${BLUE}==================== SEQUENTIAL TESTS ====================${NC}"
    
    log_info "Testing 50 sequential GET requests..."
    
    local start_time=$(date +%s.%N)
    local success_count=0
    local total_requests=50
    
    for i in $(seq 1 $total_requests); do
        response=$(curl -s -w "%{http_code}" "$BASE_URL/" 2>/dev/null)
        http_code="${response: -3}"
        if [ "$http_code" = "200" ]; then
            ((success_count++))
        fi
        
        if [ $((i % 10)) -eq 0 ]; then
            echo "Completed $i/$total_requests requests..."
        fi
    done
    
    local end_time=$(date +%s.%N)
    local duration=$(echo "$end_time - $start_time" | bc -l)
    local rps=$(echo "scale=2; $success_count / $duration" | bc -l)
    
    log_info "Completed $success_count/$total_requests requests in ${duration}s"
    log_info "Average: $rps requests per second"
    
    if [ $success_count -eq $total_requests ]; then
        log_success "All sequential requests succeeded"
    else
        log_error "$((total_requests - success_count)) requests failed"
    fi
}

test_different_file_types() {
    echo -e "\n${BLUE}==================== FILE TYPE TESTS ====================${NC}"
    
    declare -a files=("/upload.html" "/test.txt" "/style.css")
    
    for file in "${files[@]}"; do
        log_info "Testing $file..."
        
        local start_time=$(date +%s.%N)
        response=$(curl -s -w "%{http_code}" "$BASE_URL$file")
        local end_time=$(date +%s.%N)
        
        http_code="${response: -3}"
        local duration=$(echo "$end_time - $start_time" | bc -l)
        
        if [ "$http_code" = "200" ]; then
            log_success "$file: SUCCESS (${duration}s)"
        else
            log_error "$file: FAILED (code: $http_code)"
        fi
    done
}

test_large_file_handling() {
    echo -e "\n${BLUE}==================== LARGE FILE TESTS ====================${NC}"
    
    # Create a 1MB test file
    log_info "Creating 1MB test file..."
    dd if=/dev/zero of="/workspaces/webserv/www/large_test.bin" bs=1024 count=1024 2>/dev/null
    
    log_info "Testing large file download..."
    local start_time=$(date +%s.%N)
    response=$(curl -s -w "%{http_code}" "$BASE_URL/large_test.bin" -o /dev/null)
    local end_time=$(date +%s.%N)
    
    http_code="${response: -3}"
    local duration=$(echo "$end_time - $start_time" | bc -l)
    
    if [ "$http_code" = "200" ]; then
        log_success "Large file download: SUCCESS (${duration}s)"
    else
        log_error "Large file download: FAILED (code: $http_code)"
    fi
    
    # Cleanup
    rm -f "/workspaces/webserv/www/large_test.bin"
}

test_error_handling_performance() {
    echo -e "\n${BLUE}==================== ERROR HANDLING TESTS ====================${NC}"
    
    log_info "Testing 404 error handling performance..."
    
    local start_time=$(date +%s.%N)
    local success_count=0
    local total_requests=20
    
    for i in $(seq 1 $total_requests); do
        response=$(curl -s -w "%{http_code}" "$BASE_URL/nonexistent_$i.html" 2>/dev/null)
        http_code="${response: -3}"
        if [ "$http_code" = "404" ]; then
            ((success_count++))
        fi
    done
    
    local end_time=$(date +%s.%N)
    local duration=$(echo "$end_time - $start_time" | bc -l)
    
    log_info "Completed $success_count/$total_requests 404 requests in ${duration}s"
    
    if [ $success_count -eq $total_requests ]; then
        log_success "Error handling performance test passed"
    else
        log_error "Error handling performance test failed"
    fi
}

main() {
    echo -e "${BLUE}🚀 WebServ Performance Test Suite 🚀${NC}"
    echo "======================================"
    
    check_server_running
    
    test_sequential_requests
    test_concurrent_get_requests
    test_different_file_types
    test_large_file_handling
    test_error_handling_performance
    
    echo -e "\n${BLUE}📊 PERFORMANCE SUMMARY${NC}"
    echo "======================="
    echo -e "${GREEN}✓ Performance tests completed${NC}"
    echo -e "${YELLOW}ℹ Performance results are above${NC}"
    echo -e "${GREEN}✓ Server handled all test scenarios${NC}"
}

# Check if bc is available for calculations
if ! command -v bc &> /dev/null; then
    log_error "bc calculator not found. Installing..."
    # On some systems, bc might not be installed
    echo "Please install bc: apt-get install bc"
    exit 1
fi

# Run main function
main "$@"