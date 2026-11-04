// =============================================================================
// MinimalHTTPServer — single-header, single-threaded HTTP/1.1 server
// Windows only: uses WinSock2. C++17.
// =============================================================================
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

// ---------------------------------------------------------------------------
// Small helpers
// ---------------------------------------------------------------------------
inline std::string url_decode(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '+') {
            out += ' ';
        } else if (s[i] == '%' && i + 2 < s.size()) {
            int v = 0;
            std::istringstream iss(s.substr(i + 1, 2));
            iss >> std::hex >> v;
            out += static_cast<char>(v);
            i += 2;
        } else {
            out += s[i];
        }
    }
    return out;
}

inline std::string str_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

inline std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

// ---------------------------------------------------------------------------
// HTTP Request / Response
// ---------------------------------------------------------------------------
struct HTTPRequest {
    std::string method;   // "GET", "POST", …
    std::string path;     // "/api/products"
    std::string query;    // "q=laptop&page=1"
    std::string body;
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> params;   // parsed query string

    std::string get_param(const std::string& key,
                          const std::string& def = "") const {
        auto it = params.find(key);
        return it != params.end() ? it->second : def;
    }
};

struct HTTPResponse {
    int         status      = 200;
    std::string content_type = "application/json";
    std::string body;
    std::map<std::string, std::string> extra_headers;

    void set_json(const std::string& json, int code = 200) {
        status       = code;
        content_type = "application/json";
        body         = json;
    }
    void set_text(const std::string& text, int code = 200) {
        status       = code;
        content_type = "text/plain";
        body         = text;
    }
    void set_html(const std::string& html, int code = 200) {
        status       = code;
        content_type = "text/html";
        body         = html;
    }
};

// ---------------------------------------------------------------------------
// MIME type map
// ---------------------------------------------------------------------------
inline std::string mime_for(const std::string& ext) {
    static const std::unordered_map<std::string, std::string> m = {
        {".html", "text/html"},       {".htm", "text/html"},
        {".css",  "text/css"},        {".js",  "application/javascript"},
        {".json", "application/json"},{".png", "image/png"},
        {".jpg",  "image/jpeg"},      {".jpeg","image/jpeg"},
        {".gif",  "image/gif"},       {".svg", "image/svg+xml"},
        {".ico",  "image/x-icon"},    {".woff","font/woff"},
        {".woff2","font/woff2"},      {".ttf", "font/ttf"},
        {".txt",  "text/plain"},      {".xml", "application/xml"},
    };
    auto it = m.find(str_lower(ext));
    return it != m.end() ? it->second : "application/octet-stream";
}

// ---------------------------------------------------------------------------
// Handler typedefs
// ---------------------------------------------------------------------------
using Handler = std::function<void(const HTTPRequest&, HTTPResponse&)>;

// ---------------------------------------------------------------------------
// MinimalHTTPServer
// ---------------------------------------------------------------------------
class MinimalHTTPServer {
public:
    explicit MinimalHTTPServer(int port = 8080,
                               const std::string& web_root = "web")
        : port_(port), web_root_(web_root), server_sock_(INVALID_SOCKET) {
        WSADATA wsa{};
        WSAStartup(MAKEWORD(2, 2), &wsa);
    }

    ~MinimalHTTPServer() {
        if (server_sock_ != INVALID_SOCKET)
            closesocket(server_sock_);
        WSACleanup();
    }

    // Register handlers
    void Get(const std::string& path, Handler h) { get_routes_[path] = h; }
    void Post(const std::string& path, Handler h) { post_routes_[path] = h; }

    // Start listening (blocking)
    bool listen() {
        server_sock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (server_sock_ == INVALID_SOCKET) {
            std::cerr << "[HTTP] socket() failed\n";
            return false;
        }

        // Allow reuse
        int opt = 1;
        setsockopt(server_sock_, SOL_SOCKET, SO_REUSEADDR,
                   reinterpret_cast<const char*>(&opt), sizeof(opt));

        sockaddr_in addr{};
        addr.sin_family      = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port        = htons(static_cast<u_short>(port_));

        if (bind(server_sock_, reinterpret_cast<sockaddr*>(&addr),
                 sizeof(addr)) == SOCKET_ERROR) {
            std::cerr << "[HTTP] bind() failed on port " << port_ << "\n";
            return false;
        }
        if (::listen(server_sock_, SOMAXCONN) == SOCKET_ERROR) {
            std::cerr << "[HTTP] listen() failed\n";
            return false;
        }

        std::cout << "[HTTP] Listening on http://localhost:" << port_ << "\n";

        for (;;) {
            SOCKET client = accept(server_sock_, nullptr, nullptr);
            if (client == INVALID_SOCKET) continue;
            handle_client(client);
            closesocket(client);
        }
        return true;
    }

private:
    int         port_;
    std::string web_root_;
    SOCKET      server_sock_;
    std::unordered_map<std::string, Handler> get_routes_;
    std::unordered_map<std::string, Handler> post_routes_;

    // -----------------------------------------------------------------------
    // Read all available data from socket (simple: read until no more data)
    // -----------------------------------------------------------------------
    std::string recv_all(SOCKET sock) {
        std::string data;
        char buf[4096];
        // Set a short receive timeout so we don't block forever after headers
        DWORD timeout = 200; // ms
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
                   reinterpret_cast<const char*>(&timeout), sizeof(timeout));
        int n;
        while ((n = recv(sock, buf, sizeof(buf), 0)) > 0) {
            data.append(buf, n);
            // If we already have the full body based on Content-Length, stop
            size_t hdr_end = data.find("\r\n\r\n");
            if (hdr_end != std::string::npos) {
                size_t body_start = hdr_end + 4;
                // Case-insensitive search for Content-Length
                std::string ldata = data;
                for (auto& c : ldata) c = ::tolower(c);
                size_t cl_pos = ldata.find("content-length:");
                if (cl_pos != std::string::npos && cl_pos < hdr_end) {
                    size_t vstart = data.find(':', cl_pos) + 1;
                    size_t vend   = data.find("\r\n", vstart);
                    try {
                        int content_len = std::stoi(trim(data.substr(vstart, vend - vstart)));
                        if ((int)(data.size() - body_start) >= content_len)
                            break;
                    } catch (...) { break; }
                } else {
                    // No body expected
                    break;
                }
            }
        }
        return data;
    }

    // -----------------------------------------------------------------------
    // Parse raw HTTP request
    // -----------------------------------------------------------------------
    HTTPRequest parse_request(const std::string& raw) {
        HTTPRequest req;
        std::istringstream ss(raw);
        std::string line;

        // Request line
        std::getline(ss, line);
        {
            std::istringstream rl(line);
            std::string full_path;
            rl >> req.method >> full_path;

            size_t q = full_path.find('?');
            if (q != std::string::npos) {
                req.path  = full_path.substr(0, q);
                req.query = full_path.substr(q + 1);
            } else {
                req.path = full_path;
            }
            // Decode path
            req.path  = url_decode(req.path);
        }

        // Parse query params
        parse_query(req.query, req.params);

        // Headers
        while (std::getline(ss, line) && line != "\r") {
            size_t colon = line.find(':');
            if (colon != std::string::npos) {
                std::string key = str_lower(trim(line.substr(0, colon)));
                std::string val = trim(line.substr(colon + 1));
                req.headers[key] = val;
            }
        }

        // Body
        size_t body_start = raw.find("\r\n\r\n");
        if (body_start != std::string::npos) {
            req.body = raw.substr(body_start + 4);
            auto it = req.headers.find("content-length");
            if (it != req.headers.end()) {
                try {
                    size_t len = std::stoul(it->second);
                    if (req.body.size() > len) req.body = req.body.substr(0, len);
                } catch (...) {}
            }
        }

        return req;
    }

    void parse_query(const std::string& qs,
                     std::map<std::string, std::string>& out) {
        std::istringstream ss(qs);
        std::string pair;
        while (std::getline(ss, pair, '&')) {
            size_t eq = pair.find('=');
            if (eq != std::string::npos) {
                out[url_decode(pair.substr(0, eq))] =
                    url_decode(pair.substr(eq + 1));
            } else if (!pair.empty()) {
                out[url_decode(pair)] = "";
            }
        }
    }

    // -----------------------------------------------------------------------
    // Build HTTP response string
    // -----------------------------------------------------------------------
    std::string build_response(const HTTPResponse& res) {
        static const std::unordered_map<int, std::string> status_text = {
            {200, "OK"},           {201, "Created"},
            {204, "No Content"},   {400, "Bad Request"},
            {404, "Not Found"},    {405, "Method Not Allowed"},
            {500, "Internal Server Error"},
        };
        auto it = status_text.find(res.status);
        std::string stext = it != status_text.end() ? it->second : "Unknown";

        std::ostringstream out;
        out << "HTTP/1.1 " << res.status << " " << stext << "\r\n";
        out << "Content-Type: " << res.content_type << "; charset=utf-8\r\n";
        out << "Content-Length: " << res.body.size() << "\r\n";
        out << "Access-Control-Allow-Origin: *\r\n";
        out << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
        out << "Access-Control-Allow-Headers: Content-Type\r\n";
        out << "Connection: close\r\n";
        for (const auto& kv : res.extra_headers)
            out << kv.first << ": " << kv.second << "\r\n";
        out << "\r\n";
        out << res.body;
        return out.str();
    }

    // -----------------------------------------------------------------------
    // Serve a static file
    // -----------------------------------------------------------------------
    bool serve_static(const HTTPRequest& req, HTTPResponse& res) {
        // Sanitise path: prevent directory traversal
        std::string rel = req.path;
        if (rel == "/" || rel.empty()) rel = "/index.html";

        // Strip query string if present
        size_t qmark = rel.find('?');
        if (qmark != std::string::npos) rel = rel.substr(0, qmark);

        // Security: prevent directory traversal
        if (rel.find("..") != std::string::npos) return false;

        // Remove leading slash
        if (!rel.empty() && rel[0] == '/') rel = rel.substr(1);

        // Normalize slashes
        for (auto& c : rel) if (c == '/') c = '\\';

        std::string full = web_root_ + "\\" + rel;

        std::ifstream f(full, std::ios::binary);
        if (!f) return false;

        std::ostringstream buf;
        buf << f.rdbuf();
        res.body = buf.str();

        std::string ext;
        size_t dot = full.rfind('.');
        if (dot != std::string::npos) ext = full.substr(dot);
        res.content_type = mime_for(ext);
        res.status       = 200;
        return true;
    }

    // -----------------------------------------------------------------------
    // Handle one client connection
    // -----------------------------------------------------------------------
    void handle_client(SOCKET sock) {
        std::string raw = recv_all(sock);
        if (raw.empty()) return;

        HTTPRequest  req = parse_request(raw);
        HTTPResponse res;

        // Handle preflight CORS
        if (req.method == "OPTIONS") {
            res.status = 204;
            res.body   = "";
            std::string resp = build_response(res);
            send(sock, resp.c_str(), static_cast<int>(resp.size()), 0);
            return;
        }

        bool handled = false;

        // API routes
        if (req.path.rfind("/api/", 0) == 0) {
            auto& routes = (req.method == "POST") ? post_routes_ : get_routes_;
            auto it = routes.find(req.path);
            if (it != routes.end()) {
                try {
                    it->second(req, res);
                } catch (const std::exception& ex) {
                    res.set_json(
                        std::string("{\"error\":\"") + ex.what() + "\"}", 500);
                }
                handled = true;
            }
            // Prefix matching fallback (for paths like /api/graph/dfs that might vary)
            if (!handled) {
                // Try prefix: longest matching prefix
                std::string best_key;
                for (const auto& pair : routes) {
                    const std::string& k = pair.first;
                    if (req.path.rfind(k, 0) == 0 && k.size() > best_key.size())
                        best_key = k;
                }
                if (!best_key.empty()) {
                    try {
                        routes[best_key](req, res);
                    } catch (const std::exception& ex) {
                        res.set_json(
                            std::string("{\"error\":\"") + ex.what() + "\"}", 500);
                    }
                    handled = true;
                }
            }
            if (!handled) {
                res.set_json("{\"error\":\"endpoint not found\"}", 404);
            }
        }
        // Static files
        else if (req.method == "GET") {
            if (!serve_static(req, res)) {
                res.set_html("<h1>404 Not Found</h1>", 404);
            }
            handled = true;
        } else {
            res.set_json("{\"error\":\"method not allowed\"}", 405);
        }

        std::string resp = build_response(res);
        int total = static_cast<int>(resp.size());
        int sent = 0;
        while (sent < total) {
            int n = send(sock, resp.c_str() + sent, total - sent, 0);
            if (n <= 0) break;
            sent += n;
        }
        shutdown(sock, SD_SEND);
    }
};
