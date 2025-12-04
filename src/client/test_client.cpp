// ====================================================================
// LightVoice: Test Client
// src/client/test_client.cpp
//
// A simple command-line client for testing the LightVoice server.
// It uses PortAudio for cross-platform audio I/O.
//
// Author: Gemini
// ====================================================================

#include "common/Logger.h"
#include "portaudio.h"
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <atomic>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

// --- PortAudio Callback ---
static int paCallback(const void* inputBuffer, void* outputBuffer,
                      unsigned long framesPerBuffer,
                      const PaStreamCallbackTimeInfo* timeInfo,
                      PaStreamCallbackFlags statusFlags,
                      void* userData) {
    // For now, we are not playing back audio, just capturing.
    // So we just handle the input. The server will send us mixed audio.
    
    // In a real client, you would:
    // 1. Copy `inputBuffer` to a buffer to be encoded and sent.
    // 2. Copy received & decoded audio to `outputBuffer` to be played.
    
    (void)outputBuffer; // Prevent unused variable warning
    
    if (inputBuffer == NULL) {
        return paContinue;
    }

    // This is where you would take the input data and send it to the server
    const int16_t* pcm = static_cast<const int16_t*>(inputBuffer);
    // e.g., encode_and_send(pcm, framesPerBuffer);

    return paContinue;
}


int main() {
    lightvoice::Logger::Init();
    
    // --- Initialize PortAudio ---
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        LOGGER_CRITICAL("PortAudio error: {}", Pa_GetErrorText(err));
        return 1;
    }

    PaStream* stream;
    err = Pa_OpenDefaultStream(&stream,
                               NUM_CHANNELS,      // input channels
                               NUM_CHANNELS,      // output channels
                               paInt16,           // 16-bit integer
                               SAMPLE_RATE,
                               FRAMES_PER_BUFFER,
                               paCallback,
                               nullptr); // no user data

    if (err != paNoError) {
        LOGGER_CRITICAL("PortAudio error opening stream: {}", Pa_GetErrorText(err));
        Pa_Terminate();
        return 1;
    }

    // --- Network Setup ---
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        LOGGER_CRITICAL("Socket creation failed");
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8888);
    
    if(inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr) <= 0) {
        LOGGER_CRITICAL("Invalid address/ Address not supported");
        return 1;
    }

    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        LOGGER_CRITICAL("Connection failed");
        close(sock);
        return 1;
    }
    
    LOGGER_INFO("Connected to server!");

    // --- Main Loop ---
    LOGGER_INFO("Press Enter to quit.");
    std::thread network_thread([&]{
        char recvbuf[1024];
        int recvbuflen = 1024;
        while(true) {
            ssize_t result = recv(sock, recvbuf, recvbuflen, 0);
            if (result > 0) {
                LOGGER_INFO("Bytes received: {}", result);
            } else if (result == 0) {
                LOGGER_INFO("Connection closed");
                break;
            } else {
                LOGGER_WARN("recv failed: {}", strerror(errno));
                break;
            }
        }
    });


    // Start audio stream
    err = Pa_StartStream(stream);
    if (err != paNoError) {
        LOGGER_CRITICAL("PortAudio error starting stream: {}", Pa_GetErrorText(err));
    } else {
        LOGGER_INFO("Audio stream started. Speaking is ON.");
        // Main loop to keep client alive
        std::cin.get();
    }


    // --- Cleanup ---
    LOGGER_INFO("Shutting down...");
    err = Pa_StopStream(stream);
    if (err != paNoError) LOGGER_ERROR("PortAudio error stopping stream: {}", Pa_GetErrorText(err));

    err = Pa_CloseStream(stream);
    if (err != paNoError) LOGGER_ERROR("PortAudio error closing stream: {}", Pa_GetErrorText(err));
    
    Pa_Terminate();
    shutdown(sock, SHUT_RDWR);
    close(sock);
    network_thread.join();
