from http.server import HTTPServer, SimpleHTTPRequestHandler

class CacheAttackServer(SimpleHTTPRequestHandler):
    def end_headers(self):
        # Required for SharedArrayBuffer (cross-origin isolation)
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        self.send_header("Cross-Origin-Resource-Policy", "same-origin")
        super().end_headers()

if __name__ == "__main__":
    port = 8000
    server = HTTPServer(("localhost", port), CacheAttackServer)
    print(f"✅ Server ready → http://localhost:{port}")
    server.serve_forever()