#!/bin/bash

echo "Testing CGI implementation..."

# Start server in background
./webserv conf/default.conf >/dev/null 2>&1 &
SERVER_PID=$!

# Wait for server to start
sleep 1

# Initialize test results
php_test_passed=0
python_test_passed=0

# Test PHP CGI
echo "Testing PHP CGI..."
response=$(curl -s http://localhost:8080/test.php)
echo "Response: $response"

if [[ "$response" == *"CGI Script Working"* ]]; then
    echo "✅ PHP CGI test PASSED"
    php_test_passed=1
else
    echo "❌ PHP CGI test FAILED"
fi

# Test Python CGI
echo "Testing Python CGI..."
response=$(curl -s http://localhost:8080/test.py)
echo "Response: $response"

if [[ "$response" == *"Python CGI Script Working"* ]]; then
    echo "✅ Python CGI test PASSED"
    python_test_passed=1
else
    echo "❌ Python CGI test FAILED"
fi

# Stop server
kill $SERVER_PID
wait $SERVER_PID 2>/dev/null

echo "Test completed."

# Exit with error if any test failed
if [[ $php_test_passed -eq 0 || $python_test_passed -eq 0 ]]; then
    echo "Some tests failed!"
    exit 1
fi

echo "All tests passed!"
exit 0