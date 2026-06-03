/**
 * HttpServer.h - Lightweight HTTP Server using Windows Winsock API
 * 
 * Provides REST API endpoints for the Pet Simulator frontend
 * No external dependencies - uses Windows Sockets API
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <filesystem>

// Windows Sockets
#include <winsock2.h>
#include <ws2tcpip.h>
// Note: #pragma comment disabled for GCC compatibility; use -lws2_32 instead

// ============================================================
// JSON Helper Functions (No external library needed)
// ============================================================
namespace json {

inline std::string escape_string(const std::string& s) {
    std::string result;
    for (char c : s) {
        if (c == '"') result += "\\\"";
        else if (c == '\\') result += "\\\\";
        else if (c == '\n') result += "\\n";
        else if (c == '\r') result += "\\r";
        else if (c == '\t') result += "\\t";
        else result += c;
    }
    return result;
}

inline std::string quote(const std::string& s) {
    return "\"" + escape_string(s) + "\"";
}

} // namespace json

// ============================================================
// HTTP Request/Response Structures
// ============================================================
struct HttpRequest {
    std::string method;
    std::string path;
    std::string query;
    std::string body;
    std::string content_type;
};

struct HttpResponse {
    int status_code = 200;
    std::string status_text = "OK";
    std::string content_type = "application/json";
    std::string body;
    
    std::string to_string() const {
        std::ostringstream oss;
        oss << "HTTP/1.1 " << status_code << " " << status_text << "\r\n";
        oss << "Content-Type: " << content_type << "\r\n";
        oss << "Content-Length: " << body.size() << "\r\n";
        oss << "Access-Control-Allow-Origin: *\r\n";
        oss << "Connection: close\r\n";
        oss << "\r\n";
        oss << body;
        return oss.str();
    }
    
    static HttpResponse json(int code, const std::string& status, const std::string& data) {
        HttpResponse resp;
        resp.status_code = code;
        resp.status_text = status;
        resp.body = data;
        return resp;
    }
    
    static HttpResponse ok(const std::string& json_data) {
        return json(200, "OK", json_data);
    }
    
    static HttpResponse created(const std::string& json_data) {
        return json(201, "Created", json_data);
    }
    
    static HttpResponse bad_request(const std::string& msg) {
        return json(400, "Bad Request", "{\"error\":" + json::quote(msg) + "}");
    }
    
    static HttpResponse not_found(const std::string& msg) {
        return json(404, "Not Found", "{\"error\":" + json::quote(msg) + "}");
    }
    
    static HttpResponse server_error(const std::string& msg) {
        return json(500, "Internal Server Error", "{\"error\":" + json::quote(msg) + "}");
    }
};

// Route handler type
using RouteHandler = std::function<HttpResponse(const HttpRequest&)>;

// ============================================================
// Simple HTTP Server
// ============================================================
class HttpServer {
private:
    std::wstring m_port;
    std::wstring m_endpoint;
    HANDLE m_server_thread;
    bool m_running;
    std::string m_static_dir;  // Directory for static files
    
    std::vector<std::pair<std::string, RouteHandler>> m_get_routes;
    std::vector<std::pair<std::string, RouteHandler>> m_post_routes;
    std::vector<std::pair<std::string, RouteHandler>> m_put_routes;
    std::vector<std::pair<std::string, RouteHandler>> m_delete_routes;
    
    // Parse query string from URL
    static std::string parse_query(const std::string& url) {
        size_t qpos = url.find('?');
        if (qpos == std::string::npos) return "";
        return url.substr(qpos + 1);
    }
    
    // Extract path from URL
    static std::string parse_path(const std::string& url) {
        size_t qpos = url.find('?');
        if (qpos == std::string::npos) return url;
        return url.substr(0, qpos);
    }
    
    // Match route pattern with path (supports :param)
    static bool match_route(const std::string& pattern, const std::string& path,
                            std::vector<std::string>& params) {
        params.clear();

        auto p1 = pattern.begin();
        auto p2 = path.begin();

        while (p1 != pattern.end() && p2 != path.end()) {
            if (*p1 == ':') {
                // Extract parameter name and value
                ++p1;
                std::string param_name;
                while (p1 != pattern.end() && *p1 != '/' && *p1 != ':') {
                    param_name += *p1++;
                }

                std::string param_value;
                while (p2 != path.end() && *p2 != '/') {
                    param_value += *p2++;
                }

                params.push_back(param_value);

                // Skip to next segment
                while (p1 != pattern.end() && *p1 != '/') ++p1;
                while (p2 != path.end() && *p2 != '/') ++p2;
            } else if (*p1 == *p2) {
                ++p1;
                ++p2;
            } else {
                return false;
            }
        }

        // Handle trailing segments
        while (p1 != pattern.end() && *p1 == '/') ++p1;
        while (p2 != path.end() && *p2 == '/') ++p2;

        return p1 == pattern.end() && p2 == path.end();
    }
    
    // Find and call appropriate handler
    HttpResponse dispatch(const std::string& method, const std::string& path,
                         const std::string& body, const std::string& content_type) {
        HttpRequest req;
        req.method = method;
        req.path = parse_path(path);
        req.query = parse_query(path);
        req.body = body;
        req.content_type = content_type;

        std::vector<std::pair<std::string, RouteHandler>>* routes = nullptr;

        if (method == "GET") routes = &m_get_routes;
        else if (method == "POST") routes = &m_post_routes;
        else if (method == "PUT") routes = &m_put_routes;
        else if (method == "DELETE") routes = &m_delete_routes;
        else {
            return HttpResponse::bad_request("Unsupported HTTP method");
        }

        for (const auto& route : *routes) {
            std::vector<std::string> params;
            if (match_route(route.first, req.path, params)) {
                return route.second(req);
            }
        }

        return HttpResponse::not_found("Route not found: " + method + " " + path);
    }
    
    // Parse HTTP request from raw data
    static HttpRequest parse_request(const std::string& raw) {
        HttpRequest req;
        
        // HTTP uses \r\n line endings
        std::vector<std::string> lines;
        size_t pos = 0;
        while (pos < raw.size()) {
            size_t end = raw.find("\r\n", pos);
            if (end == std::string::npos) {
                // Last line might not have \r\n
                lines.push_back(raw.substr(pos));
                break;
            }
            lines.push_back(raw.substr(pos, end - pos));
            pos = end + 2;
        }
        
        // Parse request line (first line)
        if (lines.size() > 0) {
            std::istringstream line_stream(lines[0]);
            line_stream >> req.method >> req.path;
        }
        
        // Parse headers
        size_t body_start = 0;
        for (size_t i = 1; i < lines.size(); i++) {
            const std::string& line = lines[i];
            if (line.empty()) {
                // Empty line marks end of headers
                body_start = i + 1;
                break;
            }
            
            size_t colon = line.find(':');
            if (colon != std::string::npos) {
                std::string key = line.substr(0, colon);
                std::string value = line.substr(colon + 1);
                // Trim leading spaces safely to prevent std::out_of_range exception
                size_t first_non_space = value.find_first_not_of(" ");
                if (first_non_space != std::string::npos) {
                    value = value.substr(first_non_space);
                } else {
                    value = "";
                }
                // Remove trailing \r if present
                if (!value.empty() && value.back() == '\r') value.pop_back();
                
                if (key == "Content-Type") req.content_type = value;
            }
        }
        
        // Read body (remaining lines after empty line)
        if (body_start > 0 && body_start < lines.size()) {
            std::string body_content;
            for (size_t i = body_start; i < lines.size(); i++) {
                if (!body_content.empty()) body_content += "\n";
                body_content += lines[i];
            }
            req.body = body_content;
        }
        
        return req;
    }
    
    // Server thread procedure
    static DWORD WINAPI server_thread(LPVOID param) {
        HttpServer* server = reinterpret_cast<HttpServer*>(param);
        server->run_loop();
        return 0;
    }
    
    void run_loop() {
        // Initialize Winsock
        WSADATA wsaData;
        int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (wsaResult != 0) {
            return;
        }

        // Create server socket
        SOCKET server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (server_socket == INVALID_SOCKET) {
            WSACleanup();
            return;
        }

        // Set socket to non-blocking for timeout support
        u_long mode = 1;
        ioctlsocket(server_socket, FIONBIO, &mode);

        // Bind
        sockaddr_in server_addr = {0};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(static_cast<u_short>(std::stoi(m_port)));
        server_addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
            closesocket(server_socket);
            WSACleanup();
            return;
        }

        if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
            closesocket(server_socket);
            WSACleanup();
            return;
        }

        // Signal ready
        m_running = true;

        // Accept connections loop
        while (m_running) {
            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(server_socket, &read_fds);

            TIMEVAL timeout = {1, 0};  // 1 second timeout
            int select_result = select(0, &read_fds, NULL, NULL, &timeout);

            if (select_result > 0 && FD_ISSET(server_socket, &read_fds)) {
                sockaddr_in client_addr = {0};
                int addr_len = sizeof(client_addr);
                SOCKET client_socket = accept(server_socket, (sockaddr*)&client_addr, &addr_len);

                if (client_socket != INVALID_SOCKET) {
                    // Set client socket to blocking mode for reliable receive
                    u_long client_mode = 0;  // Blocking mode
                    ioctlsocket(client_socket, FIONBIO, &client_mode);
                    
                    try {
                        std::string raw_request;
                        char buffer[4096];
                        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
                        
                        if (bytes_received > 0) {
                            raw_request.append(buffer, bytes_received);
                            
                            // Parse Content-Length and loop recv if necessary to get the complete body
                            size_t header_end = raw_request.find("\r\n\r\n");
                            if (header_end != std::string::npos) {
                                size_t content_length = 0;
                                std::string headers_part = raw_request.substr(0, header_end);
                                
                                // Case-insensitive match for Content-Length
                                std::string cl_search = headers_part;
                                std::transform(cl_search.begin(), cl_search.end(), cl_search.begin(), ::tolower);
                                size_t cl_pos = cl_search.find("content-length:");
                                
                                if (cl_pos != std::string::npos) {
                                    size_t val_start = headers_part.find_first_not_of(" \t", cl_pos + 15);
                                    size_t val_end = headers_part.find("\r\n", val_start);
                                    if (val_start != std::string::npos && val_end != std::string::npos) {
                                        std::string cl_str = headers_part.substr(val_start, val_end - val_start);
                                        try {
                                            content_length = std::stoul(cl_str);
                                            // Enforce a maximum body size limit of 1MB (1,048,576 bytes) to prevent out-of-memory DoS
                                            if (content_length > 1048576) {
                                                throw std::runtime_error("Payload Too Large");
                                            }
                                        } catch (...) {
                                            throw std::runtime_error("Invalid or excessive Content-Length");
                                        }
                                    }
                                }
                                
                                // Keep reading from socket until full body is received
                                size_t body_received = raw_request.size() - (header_end + 4);
                                while (body_received < content_length) {
                                    int body_bytes = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
                                    if (body_bytes <= 0) break;
                                    raw_request.append(buffer, body_bytes);
                                    body_received += body_bytes;
                                }
                            }
                            
                            HttpRequest req = parse_request(raw_request);

                            // Dispatch to handler
                            HttpResponse resp = dispatch(req.method, req.path, req.body, req.content_type);
                            
                            // If 404 and static dir is set, try to serve static file
                            if (resp.status_code == 404 && !m_static_dir.empty()) {
                                resp = serve_static(req.path);
                            }

                            // Send response
                            std::string response_str = resp.to_string();
                            send(client_socket, response_str.c_str(), static_cast<int>(response_str.size()), 0);
                        }
                    } catch (const std::exception& e) {
                        // Safe exception handling: catch and return 500 error instead of crashing the thread
                        HttpResponse resp = HttpResponse::server_error(e.what());
                        std::string response_str = resp.to_string();
                        send(client_socket, response_str.c_str(), static_cast<int>(response_str.size()), 0);
                    }

                    closesocket(client_socket);
                }
            }
        }

        closesocket(server_socket);
        WSACleanup();
    }
    
public:
    HttpServer(int port = 8080) {
        m_port = std::to_wstring(port);
        m_running = false;
        m_server_thread = NULL;
        m_static_dir = "";
    }
    
    // Set directory for serving static files
    void set_static_dir(const std::string& dir) {
        m_static_dir = dir;
    }
    
    // Serve static file
    // Helper: check if string ends with suffix (C++17 compatible)
    static bool ends_with(const std::string& str, const std::string& suffix) {
        if (str.size() < suffix.size()) return false;
        return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
    }
    
    HttpResponse serve_static(const std::string& path) {
        // Prevent path traversal attacks (CVE-like directory traversal block)
        if (path.find("..") != std::string::npos) {
            return HttpResponse::not_found("Forbidden: Path traversal not allowed");
        }
        
        // Default file
        std::string file_path = path;
        if (path == "/" || path.empty()) {
            file_path = "/index.html";
        }
        
        // Build full path
        std::string full_path = m_static_dir + file_path;
        
        // Try to open file
        std::ifstream file(full_path, std::ios::binary);
        if (!file.is_open()) {
            return HttpResponse::not_found("File not found: " + path);
        }
        
        // Read file content safely
        std::string content;
        content.reserve(4096);  // Reserve some initial space
        std::copy(std::istreambuf_iterator<char>(file),
                  std::istreambuf_iterator<char>(),
                  std::back_inserter(content));
        
        // Determine content type
        std::string content_type = "application/octet-stream";
        if (ends_with(full_path, ".html")) content_type = "text/html; charset=utf-8";
        else if (ends_with(full_path, ".css")) content_type = "text/css; charset=utf-8";
        else if (ends_with(full_path, ".js")) content_type = "application/javascript; charset=utf-8";
        else if (ends_with(full_path, ".json")) content_type = "application/json; charset=utf-8";
        else if (ends_with(full_path, ".png")) content_type = "image/png";
        else if (ends_with(full_path, ".jpg") || ends_with(full_path, ".jpeg")) content_type = "image/jpeg";
        else if (ends_with(full_path, ".gif")) content_type = "image/gif";
        else if (ends_with(full_path, ".svg")) content_type = "image/svg+xml";
        else if (ends_with(full_path, ".ico")) content_type = "image/x-icon";
        
        // Build response
        HttpResponse resp;
        resp.status_code = 200;
        resp.status_text = "OK";
        resp.content_type = content_type;
        resp.body = content;
        
        return resp;
    }
    
    ~HttpServer() {
        stop();
    }
    
    void get(const std::string& path, RouteHandler handler) {
        m_get_routes.push_back({path, handler});
    }
    
    void post(const std::string& path, RouteHandler handler) {
        m_post_routes.push_back({path, handler});
    }
    
    void put(const std::string& path, RouteHandler handler) {
        m_put_routes.push_back({path, handler});
    }
    
    void del(const std::string& path, RouteHandler handler) {
        m_delete_routes.push_back({path, handler});
    }
    
    bool start() {
        if (m_running) return false;
        
        m_running = true;
        m_server_thread = CreateThread(NULL, 0, server_thread, this, 0, NULL);
        
        // Wait for server to be ready
        Sleep(500);
        
        return m_server_thread != NULL;
    }
    
    void stop() {
        if (!m_running) return;
        
        m_running = false;
        
        if (m_server_thread) {
            WaitForSingleObject(m_server_thread, 2000);
            CloseHandle(m_server_thread);
            m_server_thread = NULL;
        }
    }
};
