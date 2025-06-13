#!/usr/bin/python3

import os
import sys
import cgi

# Print CGI headers
print("Content-Type: text/html")
print()  # Empty line to end headers

print("<html><head><title>Python CGI Test</title></head><body>")
print("<h1>Python CGI Script Working!</h1>")

print("<h2>Environment Variables:</h2>")
print("<ul>")
for var in ['REQUEST_METHOD', 'PATH_INFO', 'QUERY_STRING', 'CONTENT_TYPE', 'CONTENT_LENGTH']:
    value = os.environ.get(var, 'not set')
    print(f"<li>{var}: {value}</li>")
print("</ul>")

# Handle POST data
if os.environ.get('REQUEST_METHOD') == 'POST':
    print("<h2>POST Data:</h2>")
    content_length = int(os.environ.get('CONTENT_LENGTH', '0'))
    if content_length > 0:
        post_data = sys.stdin.read(content_length)
        print(f"<pre>{cgi.escape(post_data)}</pre>")

# Handle query string
query_string = os.environ.get('QUERY_STRING', '')
if query_string:
    print("<h2>Query String:</h2>")
    print(f"<pre>{cgi.escape(query_string)}</pre>")

print("</body></html>")