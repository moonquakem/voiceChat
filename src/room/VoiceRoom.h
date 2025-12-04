// ====================================================================
// LightVoice: Voice Room
// src/room/VoiceRoom.h
//
// Represents a single voice chat room. It manages members, handles
// audio packet routing, and owns the AudioMixer for the room.
//
// Author: Gemini
// ====================================================================

#pragma once

#include "common/noncopyable.h"
#include "codec/AudioMixer.h"
#include <cstdint>
#include <string>
#include <map>
#include <memory>
#include <mutex>

namespace lightvoice {

class User; // Forward declaration
using UserPtr = std::shared_ptr<User>;

class VoiceRoom : noncopyable, public std::enable_shared_from_this<VoiceRoom> {
public:
    VoiceRoom(uint32_t id, std::string name, UserPtr owner);
    ~VoiceRoom();

    void start();
    void stop();

    void addUser(UserPtr user);
    void removeUser(UserPtr user);
    
    void onAudioPacket(uint32_t userId, AudioFramePtr frame);
    
    void broadcastMessage(const google::protobuf::Message& message);

    uint32_t id() const { return id_; }
    const std::string& name() const { return name_; }

private:
    void onMixTimer();

    uint32_t id_;
    std::string name_;
    UserPtr owner_;
    
    std::mutex mutex_;
    std::map<uint32_t, UserPtr> members_;
    
    std::unique_ptr<AudioMixer> mixer_;
    
    // Frames received in the last 20ms interval, waiting to be mixed.
    std::vector<AudioFramePtr> pending_frames_;
};

using VoiceRoomPtr = std::shared_ptr<VoiceRoom>;

} // namespace lightvoice
