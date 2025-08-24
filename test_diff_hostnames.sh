#!/bin/bash

# Virtual Hosting Test Script
# Tests multiple virtual hosts with different hostnames on the same IP

echo "🔍 Testing Virtual Hosting Functionality"
echo "========================================"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Test counter
TESTS_PASSED=0
TESTS_FAILED=0

# Helper function to run test
run_test() {
    local test_name="$1"
    local curl_cmd="$2"
    local expected_text="$3"
    
    echo -e "\n${BLUE}Testing: $test_name${NC}"
    echo "Command: $curl_cmd"
    
    # Run curl and capture output
    response=$(eval "$curl_cmd" 2>/dev/null)
    exit_code=$?
    
    if [ $exit_code -eq 0 ] && [[ "$response" == *"$expected_text"* ]]; then
        echo -e "${GREEN}✅ PASSED${NC} - Found expected text: '$expected_text'"
        ((TESTS_PASSED++))
    else
        echo -e "${RED}❌ FAILED${NC} - Expected text not found: '$expected_text'"
        if [ $exit_code -ne 0 ]; then
            echo -e "${RED}   Curl exit code: $exit_code${NC}"
        else
            echo -e "${YELLOW}   Response excerpt:${NC}"
            echo "$response" | head -5
        fi
        ((TESTS_FAILED++))
    fi
}

echo -e "${YELLOW}Note: Make sure WebServ is running with conf/eval.conf${NC}"
echo ""

# Test 1: naruto.com homepage
run_test "naruto.com - Homepage" \
    "curl -s --resolve naruto.com:3001:127.0.0.1 http://naruto.com:3001/" \
    "Welcome to Naruto.com"

# Test 2: naruto.com about page
run_test "naruto.com - About page" \
    "curl -s --resolve naruto.com:3001:127.0.0.1 http://naruto.com:3001/about.html" \
    "About Naruto.com"

# Test 3: sasuke.com homepage
run_test "sasuke.com - Homepage" \
    "curl -s --resolve sasuke.com:3002:127.0.0.1 http://sasuke.com:3002/" \
    "Sasuke.com"

# Test 4: sasuke.com team page
run_test "sasuke.com - Team page" \
    "curl -s --resolve sasuke.com:3002:127.0.0.1 http://sasuke.com:3002/team.html" \
    "Team Sasuke"

# Test 5: localhost default host
run_test "localhost - Default host" \
    "curl -s http://localhost:8080/" \
    "WebServ Default Host"

# Test 6: localhost admin page
run_test "localhost - Admin page" \
    "curl -s http://localhost:8080/admin.html" \
    "WebServ Administration"

# Test 7: CSS serving for naruto.com
run_test "naruto.com - CSS file" \
    "curl -s --resolve naruto.com:3001:127.0.0.1 http://naruto.com:3001/style.css" \
    "Naruto.com Virtual Host Styles"

# Test 8: CSS serving for sasuke.com
run_test "sasuke.com - CSS file" \
    "curl -s --resolve sasuke.com:3002:127.0.0.1 http://sasuke.com:3002/style.css" \
    "Sasuke.com Virtual Host Styles"

# Test 9: Different content verification - check that hosts serve different content
echo -e "\n${BLUE}Testing: Content Differentiation${NC}"
naruto_response=$(curl -s --resolve naruto.com:3001:127.0.0.1 http://naruto.com:3001/ 2>/dev/null)
sasuke_response=$(curl -s --resolve sasuke.com:3002:127.0.0.1 http://sasuke.com:3002/ 2>/dev/null)

if [[ "$naruto_response" != "$sasuke_response" ]] && [[ -n "$naruto_response" ]] && [[ -n "$sasuke_response" ]]; then
    echo -e "${GREEN}✅ PASSED${NC} - Different hosts serve different content"
    ((TESTS_PASSED++))
else
    echo -e "${RED}❌ FAILED${NC} - Hosts serve identical content or no response"
    ((TESTS_FAILED++))
fi

# Summary
echo -e "\n${YELLOW}========================================${NC}"
echo -e "${YELLOW}Virtual Hosting Test Summary${NC}"
echo -e "${YELLOW}========================================${NC}"
echo -e "${GREEN}Tests Passed: $TESTS_PASSED${NC}"
echo -e "${RED}Tests Failed: $TESTS_FAILED${NC}"

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "\n${GREEN}🎉 All tests passed! Virtual hosting is working correctly.${NC}"
    exit 0
else
    echo -e "\n${RED}⚠️  Some tests failed. Check WebServ configuration and server status.${NC}"
    exit 1
fi
