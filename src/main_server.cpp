/**
 * Pet Simulator HTTP Server
 *
 * REST API server for the Pet Simulator frontend
 * Uses Windows Sockets for HTTP server functionality
 *
 * Usage: PetSimulatorServer.exe [port]
 * Default port: 8080
 */

#include <iostream>
#include <string>
#include <memory>
#include <csignal>

#include "HttpServer.h"
#include "GameState.h"
#include "ApiController.h"

#ifdef _WIN32
#include <windows.h>
BOOL WINAPI console_handler(DWORD dwType) {
    if (dwType == CTRL_C_EVENT || dwType == CTRL_BREAK_EVENT) {
        // std::cout << "\nShutting down server...\n";
        GameState::destroy();
        exit(0);
    }
    return TRUE;
}
#else
void console_handler(int) {
    // std::cout << "\nShutting down server...\n";
    GameState::destroy();
    exit(0);
}
#endif

void print_banner() {
    // Banner printing removed for background execution
    return;
}

void print_usage(const char* program) {
    // std::cout << "Usage: " << program << " [port]\n";
    // std::cout << "  port  - HTTP server port (default: 8080)\n";
    // std::cout << "\nExample:\n";
    // std::cout << "  " << program << " 8080\n";
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    // Set console to UTF-8 to avoid garbled output
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    // Parse command line
    int port = 8080;
    if (argc > 1) {
        if (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help") {
            print_usage(argv[0]);
            return 0;
        }
        port = std::atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            // std::cerr << "Error: Invalid port number\n";
            print_usage(argv[0]);
            return 1;
        }
    }

    // Setup console handler for graceful shutdown
#ifdef _WIN32
    SetConsoleCtrlHandler(console_handler, TRUE);
#else
    signal(SIGINT, console_handler);
    signal(SIGTERM, console_handler);
#endif

    // Print banner
    // print_banner();

    // Initialize game state
    // std::cout << "\n[INFO] Initializing game state...\n";
    GameState& game = GameState::get_instance();
    // std::cout << "[INFO] Game state ready!\n";

    // Create HTTP server
    // std::cout << "[INFO] Starting HTTP server on port " << port << "...\n";
    HttpServer server(port);

    // Set static files directory to web/ subdirectory
    // This makes /index.html → ./web/index.html, /style.css → ./web/style.css
    server.set_static_dir("./web");

    // Create API controller and register routes (must persist for server lifetime)
    std::unique_ptr<ApiController> controller(new ApiController(game));
    controller->register_routes(server);

    // Debug: print registered routes
    // Enable this temporarily for debugging
    // std::cout << "Server routes registered. Try: curl http://localhost:" << port << "/api/monsters/random?difficulty=3\n";

    // Start server
    if (!server.start()) {
        // std::cerr << "[ERROR] Failed to start server. Port " << port << " may be in use.\n";
        GameState::destroy();
        return 1;
    }

    // std::cout << "[INFO] Server started successfully!\n";
    // std::cout << "[INFO] Waiting for requests...\n\n";

    // Keep server running
    while (true) {
        Sleep(1000);
        // Process any pending actions
    }

    // Cleanup (never reached)
    server.stop();
    GameState::destroy();
    return 0;
}
