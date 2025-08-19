#!/usr/bin/env python3
import cgi

# Captura o dado enviado pelo usuário
form = cgi.FieldStorage()
word = form.getfirst("data", "")
word_upper = word.upper()

# Cabeçalho HTTP
print("Content-Type: text/html; charset=utf-8")
print()

print(f"""<!DOCTYPE html>
<html>
<head>
    <title>CGI Test - Resultado</title>
    <link rel="stylesheet" href="/style.css" />
</head>
<body>
<nav class="navbar">
  <ul class="nav-links">
    <li><a href="index.html">Home</a></li>
    <li><a href="upload.html">Upload</a></li>
    <li><a href="cgi.html">CGI Test</a></li>
  </ul>
</nav>

<h1>CGI Test - Resultado</h1>
<div class="upload-form">
    <h2>Resultado do texto enviado</h2>
    <p><strong>Você digitou:</strong> {word}</p>
    <p><strong>Em caixa alta:</strong> {word_upper}</p>
    <br>
    <a href="/cgi.html"><button>Voltar</button></a>
</div>
</body>
</html>
""")
