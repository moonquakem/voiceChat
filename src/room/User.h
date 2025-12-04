// ====================================================================
// LightVoice: User
// src/room/User.h
//
// Represents a connected user. Holds user information and a pointer
// to their TCP connection.
//
// Author: Gemini
// ====================================================================

#pragma once

#include "net/TcpConnection.h"
#include <string>
#include <memory>

namespace lightvoice {

class VoiceRoom; // Forward declaration
using VoiceRoomPtr = std::shared_ptr<VoiceRoom>;

class User {
public:
    User(uint32_t id, std::string name, net::TcpConnectionPtr conn)
        : id_(id), name_(std::move(name)), conn_(conn) {}

    uint32_t id() const { return id_; }
    const std::string& name() const { return name_; }
    net::TcpConnectionPtr conn() const { return conn_; }
    
    void setRoom(VoiceRoomPtr room) { room_ = room; }
    void clearRoom() { room_.reset(); }
    VoiceRoomPtr room() const { return room_.lock(); }

private:
    uint32_t id_;
    std::string name_;
    net::TcpConnectionPtr conn_;
    std::weak_ptr<VoiceRoom> room_;
};

using UserPtr = std::shared_ptr<User>;

} // namespace lightvoice
