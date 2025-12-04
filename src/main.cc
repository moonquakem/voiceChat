// ====================================================================
// LightVoice: Main Server Entry Point
// src/main.cc
//
// This file contains the main function for the LightVoice server.
// It sets up the server, defines callbacks, and starts the event loop.
//
// Author: Gemini
// ====================================================================

#include "common/Logger.h"
#include "net/EventLoop.h"
#include "net/TcpServer.h"
#include "net/InetAddress.h"
#include "proto/chat.pb.h"
#include <iostream>

using namespace lightvoice;
using namespace lightvoice::net;

// A simple connection callback
void onConnection(const TcpConnectionPtr& conn) {
    if (conn->connected()) {
        LOGGER_INFO("New connection {} from {}", conn->name(), conn->peerAddress().toIpPort());
    } else {
        LOGGER_INFO("Connection {} is down", conn->name());
    }
}

// A simple message callback (just logs the message)
void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time) {
    LOGGER_DEBUG("Received {} bytes from {}", buf->readableBytes(), conn->name());
    
    // For now, just retrieve all data to clear the buffer
    std::string msg = buf->retrieveAllAsString();
    LOGGER_INFO("Message content: {}", msg);

    // In a real app, you would parse the protobuf message here
    // e.g., using the ProtobufCodec
}

int main(int argc, char* argv[]) {
    // Initialize the logger
    Logger::Init();
    LOGGER_INFO("Starting LightVoice Server...");

    // Check for port argument
    uint16_t port = 8888;
    if (argc > 1) {
        port = static_cast<uint16_t>(atoi(argv[1]));
    }
    LOGGER_INFO("Listening on port {}", port);

    // The main event loop
    EventLoop loop;

    // The server address
    InetAddress listenAddr(port);

    // The TcpServer
    TcpServer server(&loop, listenAddr, "LightVoiceServer");

    // Set callbacks
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);

    // Set the number of I/O threads
    server.setThreadNum(4); // e.g., 4 IO threads

    // Start the server
    server.start();

    // Start the main event loop
    loop.loop();

    return 0;
}
