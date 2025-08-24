#!/bin/bash

# Comprehensive Virtual Hosting Test Suite
# Advanced testing for WebServ virtual hosting functionality

echo "🚀 Comprehensive Virtual Hosting Test Suite"
echo "============================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
NC='\033[0m'

# Test counters
TESTS_PASSED=0
TESTS_FAILED=0
TOTAL_TESTS=0

# Configuration
TIMEOUT=5
USER_AGENT="WebServ-VHost-Test/1.0"

# Helper function to run test with enhanced error handling
run_advanced_test() {
    local test_name="$1"
    local curl_cmd="$2"
    local expected_text="$3"
    local should_fail="${4:-false}"
    
    ((TOTAL_TESTS++))
    echo -e "\n${CYAN}[Test $TOTAL_TESTS]${NC} ${BLUE}$test_name${NC}"
    echo "Command: $curl_cmd"
    
    # Add timeout and user agent to curl command
    enhanced_cmd=$(echo "$curl_cmd" | sed "s/curl/curl --max-time $TIMEOUT --user-agent '$USER_AGENT'/")
    
    # Run curl and capture both stdout and stderr
    response=$(eval "$enhanced_cmd" 2>&1)
    exit_code=$?
    
    # Check if test should fail
    if [ "$should_fail" == "true" ]; then
        if [ $exit_code -ne 0 ] || [[ "$response" != *"$expected_text"* ]]; then
            echo -e "${GREEN}✅ PASSED${NC} - Expected failure occurred"
            ((TESTS_PASSED++))
        else
            echo -e "${RED}❌ FAILED${NC} - Expected failure but test succeeded"
            ((TESTS_FAILED++))
        fi
    else
        if [ $exit_code -eq 0 ] && [[ "$response" == *"$expected_text"* ]]; then
            echo -e "${GREEN}✅ PASSED${NC} - Found expected text: '$expected_text'"
            ((TESTS_PASSED++))
        else
            echo -e "${RED}❌ FAILED${NC} - Expected text not found: '$expected_text'"
            if [ $exit_code -ne 0 ]; then
                echo -e "${RED}   Curl exit code: $exit_code${NC}"
            fi
            echo -e "${YELLOW}   Response preview:${NC}"
            echo "$response" | head -3 | sed 's/^/   /'
            ((TESTS_FAILED++))
        fi
    fi
}

# Function to test response headers
test_response_headers() {
    local test_name="$1"
    local url="$2"
    local expected_header="$3"
    
    ((TOTAL_TESTS++))
    echo -e "\n${CYAN}[Test $TOTAL_TESTS]${NC} ${BLUE}$test_name${NC}"
    
    headers=$(curl -s -I --max-time $TIMEOUT "$url" 2>/dev/null)
    exit_code=$?
    
    if [ $exit_code -eq 0 ] && [[ "$headers" == *"$expected_header"* ]]; then
        echo -e "${GREEN}✅ PASSED${NC} - Found expected header: '$expected_header'"
        ((TESTS_PASSED++))
    else
        echo -e "${RED}❌ FAILED${NC} - Expected header not found: '$expected_header'"
        echo -e "${YELLOW}   Headers received:${NC}"
        echo "$headers" | head -5 | sed 's/^/   /'
        ((TESTS_FAILED++))
    fi
}

# Function to compare response times
test_performance() {
    local test_name="$1"
    local url1="$2"
    local url2="$3"
    
    ((TOTAL_TESTS++))
    echo -e "\n${CYAN}[Test $TOTAL_TESTS]${NC} ${BLUE}$test_name${NC}"
    
    time1=$(curl -w "%{time_total}" -s -o /dev/null --max-time $TIMEOUT "$url1" 2>/dev/null)
    time2=$(curl -w "%{time_total}" -s -o /dev/null --max-time $TIMEOUT "$url2" 2>/dev/null)
    
    if [ -n "$time1" ] && [ -n "$time2" ]; then
        echo -e "${GREEN}✅ PASSED${NC} - Response times: $url1 (${time1}s), $url2 (${time2}s)"
        ((TESTS_PASSED++))
    else
        echo -e "${RED}❌ FAILED${NC} - Could not measure response times"
        ((TESTS_FAILED++))
    fi
}

echo -e "${YELLOW}Prerequisites: WebServ must be running with conf/eval.conf${NC}"
echo -e "${YELLOW}Configuration: 3 virtual hosts on localhost:8080, naruto.com:3001, sasuke.com:3002${NC}\n"

# ============================================================================
# SECTION 1: Basic Virtual Host Resolution
# ============================================================================
echo -e "${MAGENTA}█ SECTION 1: Basic Virtual Host Resolution${NC}"

run_advanced_test "naruto.com - Homepage" \
    "curl -s --resolve naruto.com:3001:127.0.0.1 http://naruto.com:3001/" \
    "Welcome to Naruto.com"

run_advanced_test "sasuke.com - Homepage" \
    "curl -s --resolve sasuke.com:3002:127.0.0.1 http://sasuke.com:3002/" \
    "Sasuke.com"

run_advanced_test "localhost - Default host" \
    "curl -s http://localhost:8080/" \
    "WebServ Default Host"

# ============================================================================
# SECTION 2: Subpage Content Testing
# ============================================================================
echo -e "\n${MAGENTA}█ SECTION 2: Subpage Content Testing${NC}"

run_advanced_test "naruto.com - About page" \
    "curl -s --resolve naruto.com:3001:127.0.0.1 http://naruto.com:3001/about.html" \
    "Virtual Host Testing"

run_advanced_test "sasuke.com - Team page" \
    "curl -s --resolve sasuke.com:3002:127.0.0.1 http://sasuke.com:3002/team.html" \
    "Virtual Host Validation"

run_advanced_test "localhost - Admin page" \
    "curl -s http://localhost:8080/admin.html" \
    "WebServ Administration"

# ============================================================================
# SECTION 3: Static Asset Serving
# ============================================================================
echo -e "\n${MAGENTA}█ SECTION 3: Static Asset Serving${NC}"

run_advanced_test "naruto.com - CSS file" \
    "curl -s --resolve naruto.com:3001:127.0.0.1 http://naruto.com:3001/style.css" \
    "Naruto.com Virtual Host Styles"

run_advanced_test "sasuke.com - CSS file" \
    "curl -s --resolve sasuke.com:3002:127.0.0.1 http://sasuke.com:3002/style.css" \
    "Sasuke.com Virtual Host Styles"

run_advanced_test "localhost - CSS file" \
    "curl -s http://localhost:8080/style.css" \
    "Localhost Default Virtual Host Styles"

# ============================================================================
# SECTION 4: Content Differentiation Validation
# ============================================================================
echo -e "\n${MAGENTA}█ SECTION 4: Content Differentiation Validation${NC}"

((TOTAL_TESTS++))
echo -e "\n${CYAN}[Test $TOTAL_TESTS]${NC} ${BLUE}Content Uniqueness - naruto.com vs sasuke.com${NC}"
naruto_content=$(curl -s --resolve naruto.com:3001:127.0.0.1 http://naruto.com:3001/ 2>/dev/null)
sasuke_content=$(curl -s --resolve sasuke.com:3002:127.0.0.1 http://sasuke.com:3002/ 2>/dev/null)

if [[ "$naruto_content" != "$sasuke_content" ]] && [[ -n "$naruto_content" ]] && [[ -n "$sasuke_content" ]]; then
    echo -e "${GREEN}✅ PASSED${NC} - Virtual hosts serve unique content"
    ((TESTS_PASSED++))
else
    echo -e "${RED}❌ FAILED${NC} - Virtual hosts serve identical content"
    ((TESTS_FAILED++))
fi

((TOTAL_TESTS++))
echo -e "\n${CYAN}[Test $TOTAL_TESTS]${NC} ${BLUE}Content Uniqueness - localhost vs naruto.com${NC}"
localhost_content=$(curl -s http://localhost:8080/ 2>/dev/null)

if [[ "$localhost_content" != "$naruto_content" ]] && [[ -n "$localhost_content" ]]; then
    echo -e "${GREEN}✅ PASSED${NC} - localhost serves different content than naruto.com"
    ((TESTS_PASSED++))
else
    echo -e "${RED}❌ FAILED${NC} - localhost serves identical content to naruto.com"
    ((TESTS_FAILED++))
fi

# ============================================================================
# SECTION 5: HTTP Header Testing
# ============================================================================
echo -e "\n${MAGENTA}█ SECTION 5: HTTP Header Testing${NC}"

test_response_headers "naruto.com - HTTP Status" \
    "--resolve naruto.com:3001:127.0.0.1 http://naruto.com:3001/" \
    "HTTP/1.1 200"

test_response_headers "sasuke.com - HTTP Status" \
    "--resolve sasuke.com:3002:127.0.0.1 http://sasuke.com:3002/" \
    "HTTP/1.1 200"

test_response_headers "localhost - HTTP Status" \
    "http://localhost:8080/" \
    "HTTP/1.1 200"

# ============================================================================
# SECTION 6: Error Handling
# ============================================================================
echo -e "\n${MAGENTA}█ SECTION 6: Error Handling${NC}"

run_advanced_test "naruto.com - 404 Error" \
    "curl -s --resolve naruto.com:3001:127.0.0.1 http://naruto.com:3001/nonexistent.html" \
    "404" \
    "false"

run_advanced_test "Invalid hostname fallback" \
    "curl -s --resolve invalid.com:3001:127.0.0.1 http://invalid.com:3001/" \
    "" \
    "true"

# ============================================================================
# SECTION 7: Performance Testing
# ============================================================================
echo -e "\n${MAGENTA}█ SECTION 7: Performance Testing${NC}"

test_performance "Response time comparison" \
    "--resolve naruto.com:3001:127.0.0.1 http://naruto.com:3001/" \
    "--resolve sasuke.com:3002:127.0.0.1 http://sasuke.com:3002/"

# ============================================================================
# SECTION 8: Advanced Virtual Host Features
# ============================================================================
echo -e "\n${MAGENTA}█ SECTION 8: Advanced Virtual Host Features${NC}"

# Test with different HTTP methods
run_advanced_test "naruto.com - POST method support" \
    "curl -s -X POST --resolve naruto.com:3001:127.0.0.1 http://naruto.com:3001/" \
    "Naruto.com"

# Test case sensitivity
run_advanced_test "Case-insensitive hostname (if supported)" \
    "curl -s --resolve NARUTO.COM:3001:127.0.0.1 http://NARUTO.COM:3001/" \
    "Naruto.com" \
    "false"

# ============================================================================
# FINAL RESULTS
# ============================================================================
echo -e "\n${YELLOW}============================================${NC}"
echo -e "${YELLOW}  COMPREHENSIVE TEST RESULTS${NC}"
echo -e "${YELLOW}============================================${NC}"
echo -e "${GREEN}Tests Passed: $TESTS_PASSED${NC}"
echo -e "${RED}Tests Failed: $TESTS_FAILED${NC}"
echo -e "${BLUE}Total Tests:  $TOTAL_TESTS${NC}"

if [ $TESTS_FAILED -eq 0 ]; then
    success_rate=100
    echo -e "\n${GREEN}🎉 ALL TESTS PASSED! Virtual hosting is fully functional.${NC}"
    echo -e "${GREEN}Success Rate: $success_rate%${NC}"
else
    success_rate=$((TESTS_PASSED * 100 / TOTAL_TESTS))
    echo -e "\n${YELLOW}⚠️  Some tests failed. Success Rate: $success_rate%${NC}"
    if [ $success_rate -ge 80 ]; then
        echo -e "${YELLOW}Virtual hosting is mostly working but needs attention.${NC}"
    else
        echo -e "${RED}Virtual hosting has significant issues that need fixing.${NC}"
    fi
fi

echo -e "\n${CYAN}Test completed at: $(date)${NC}"

exit $TESTS_FAILED