<?php
header("Content-Type: text/html");

echo "<html><head><title>CGI Test</title></head><body>";
echo "<h1>CGI Script Working!</h1>";
echo "<h2>Environment Variables:</h2>";
echo "<ul>";
echo "<li>REQUEST_METHOD: " . (getenv('REQUEST_METHOD') ?: 'not set') . "</li>";
echo "<li>PATH_INFO: " . (getenv('PATH_INFO') ?: 'not set') . "</li>";
echo "<li>QUERY_STRING: " . (getenv('QUERY_STRING') ?: 'not set') . "</li>";
echo "<li>CONTENT_TYPE: " . (getenv('CONTENT_TYPE') ?: 'not set') . "</li>";
echo "<li>CONTENT_LENGTH: " . (getenv('CONTENT_LENGTH') ?: 'not set') . "</li>";
echo "</ul>";

if (getenv('REQUEST_METHOD') === 'POST') {
    echo "<h2>POST Data:</h2>";
    $input = file_get_contents('php://stdin');
    echo "<pre>" . htmlspecialchars($input) . "</pre>";
}

if (getenv('QUERY_STRING')) {
    echo "<h2>Query String:</h2>";
    echo "<pre>" . htmlspecialchars(getenv('QUERY_STRING')) . "</pre>";
}

echo "</body></html>";
?>