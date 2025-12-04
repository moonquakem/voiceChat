// ====================================================================
// LightVoice: Stress Test
// benchmark/stress_test.cpp
//
// A simple stress test client that creates a large number of
// concurrent connections to the server.
//
// Author: Gemini
// ====================================================================

#include "common/Logger.h"
#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <cstring>

std::atomic<int> connected_clients(0);
std::atomic<bool> stop_test(false);

void client_thread_func(const char* ip, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return;

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    
    if(inet_pton(AF_INET, ip, &serverAddr.sin_addr) <= 0) {
        close(sock);
        return;
    }

    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        close(sock);
        return;
    }
    
    connected_clients++;

    while (!stop_test) {
        // Keep the connection alive
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        char buf[] = "heartbeat";
        send(sock, buf, sizeof(buf), 0);
    }

    shutdown(sock, SHUT_RDWR);
    close(sock);
}

int main(int argc, char* argv[]) {
    lightvoice::Logger::Init();

    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <ip> <port> <num_clients>" << std::endl;
        return 1;
    }

    const char* ip = argv[1];
    int port = std::stoi(argv[2]);
    int num_clients = std::stoi(argv[3]);

    LOGGER_INFO("Starting stress test for {} clients on {}:{}", num_clients, ip, port);

    std::vector<std::thread> threads;
    for (int i = 0; i < num_clients; ++i) {
        threads.emplace_back(client_thread_func, ip, port);
        // Stagger connections slightly
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    
    LOGGER_INFO("All client threads launched. Waiting for connections...");

    while(connected_clients < num_clients) {
        LOGGER_INFO("Connected clients: {}/{}", connected_clients.load(), num_clients);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (connected_clients == num_clients) break;
    }
    
    LOGGER_INFO("All clients connected! Running test. Press Enter to stop.");
    std::cin.get();

    stop_test = true;

    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    
    LOGGER_INFO("Stress test finished.");
    return 0;
}
