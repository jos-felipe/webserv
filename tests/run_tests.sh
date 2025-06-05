#!/bin/bash

# WebServ Test Runner
# Runs both unit tests and integration tests

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
SERVER_PID=""

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[PASS]${NC} $1"
}

log_error() {
    echo -e "${RED}[FAIL]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

cleanup() {
    if [ ! -z "$SERVER_PID" ]; then
        log_info "Stopping WebServ server (PID: $SERVER_PID)..."
        kill $SERVER_PID 2>/dev/null || true
        wait $SERVER_PID 2>/dev/null || true
    fi
}

# Set up cleanup trap
trap cleanup EXIT

build_project() {
    log_info "Building WebServ..."
    cd "$PROJECT_DIR"
    
    if make clean && make; then
        log_success "Build successful"
    else
        log_error "Build failed"
        exit 1
    fi
}

build_unit_tests() {
    log_info "Building unit tests..."
    cd "$PROJECT_DIR"
    
    if c++ -Wall -Wextra -Werror -std=c++98 -Iinclude \
        tests/test_unit.cpp \
        obj/config/Config.o \
        obj/http/HttpRequest.o \
        obj/http/HttpResponse.o \
        obj/socket/Socket.o \
        -o tests/test_unit; then
        log_success "Unit tests build successful"
    else
        log_error "Unit tests build failed"
        exit 1
    fi
}

run_unit_tests() {
    log_info "Running unit tests..."
    cd "$PROJECT_DIR"
    
    if ./tests/test_unit; then
        log_success "Unit tests passed"
        return 0
    else
        log_error "Unit tests failed"
        return 1
    fi
}

start_server() {
    log_info "Starting WebServ server..."
    cd "$PROJECT_DIR"
    
    # Start server in background
    ./webserv conf/default.conf > /tmp/webserv_test.log 2>&1 &
    SERVER_PID=$!
    
    # Wait for server to start
    sleep 2
    
    # Check if server is running
    if kill -0 $SERVER_PID 2>/dev/null; then
        log_success "WebServ server started (PID: $SERVER_PID)"
        
        # Additional check: try to connect
        if curl -s --connect-timeout 5 "http://localhost:8080" > /dev/null 2>&1; then
            log_success "Server is accepting connections"
            return 0
        else
            log_error "Server is not accepting connections"
            return 1
        fi
    else
        log_error "Failed to start WebServ server"
        return 1
    fi
}

run_integration_tests() {
    log_info "Running integration tests..."
    cd "$PROJECT_DIR"
    
    if ./tests/test_webserv.sh; then
        log_success "Integration tests passed"
        return 0
    else
        log_error "Integration tests failed"
        return 1
    fi
}

show_server_logs() {
    if [ -f "/tmp/webserv_test.log" ]; then
        log_info "Server logs:"
        echo "----------------------------------------"
        tail -20 /tmp/webserv_test.log
        echo "----------------------------------------"
    fi
}

print_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -u, --unit-only      Run only unit tests"
    echo "  -i, --integration-only   Run only integration tests"
    echo "  -s, --skip-build     Skip building (use existing binaries)"
    echo "  -l, --show-logs      Show server logs after tests"
    echo "  -h, --help           Show this help message"
    echo ""
    echo "Default: Run all tests (unit + integration)"
}

main() {
    local unit_only=false
    local integration_only=false
    local skip_build=false
    local show_logs=false
    
    # Parse command line arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            -u|--unit-only)
                unit_only=true
                shift
                ;;
            -i|--integration-only)
                integration_only=true
                shift
                ;;
            -s|--skip-build)
                skip_build=true
                shift
                ;;
            -l|--show-logs)
                show_logs=true
                shift
                ;;
            -h|--help)
                print_usage
                exit 0
                ;;
            *)
                log_error "Unknown option: $1"
                print_usage
                exit 1
                ;;
        esac
    done
    
    echo -e "${BLUE}🚀 WebServ Test Suite 🚀${NC}"
    echo "================================"
    
    local all_passed=true
    
    # Build project
    if [ "$skip_build" = false ]; then
        build_project
    else
        log_warning "Skipping build as requested"
    fi
    
    # Run unit tests
    if [ "$integration_only" = false ]; then
        echo -e "\n${BLUE}📋 UNIT TESTS${NC}"
        echo "==================="
        
        if [ "$skip_build" = false ]; then
            build_unit_tests
        fi
        
        if ! run_unit_tests; then
            all_passed=false
        fi
    fi
    
    # Run integration tests
    if [ "$unit_only" = false ]; then
        echo -e "\n${BLUE}🌐 INTEGRATION TESTS${NC}"
        echo "======================="
        
        if start_server; then
            if ! run_integration_tests; then
                all_passed=false
            fi
        else
            log_error "Cannot run integration tests without server"
            all_passed=false
        fi
        
        if [ "$show_logs" = true ]; then
            show_server_logs
        fi
    fi
    
    # Final summary
    echo -e "\n${BLUE}📊 FINAL SUMMARY${NC}"
    echo "=================="
    
    if [ "$all_passed" = true ]; then
        echo -e "${GREEN}🎉 ALL TESTS PASSED! 🎉${NC}"
        echo -e "${GREEN}WebServ enhancements are working correctly.${NC}"
        exit 0
    else
        echo -e "${RED}❌ SOME TESTS FAILED ❌${NC}"
        echo -e "${RED}Please check the output above for details.${NC}"
        exit 1
    fi
}

# Run main function with all arguments
main "$@"