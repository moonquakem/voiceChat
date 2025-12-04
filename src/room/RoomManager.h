// ====================================================================
// LightVoice: Room Manager
// src/room/RoomManager.h
//
// A singleton manager for all voice rooms. It handles creating,
// finding, and destroying rooms.
//
// Author: Gemini
// ====================================================================

#pragma once

#include "common/noncopyable.h"
#include "room/VoiceRoom.h"
#include <map>
#include <mutex>

namespace lightvoice {

class RoomManager : noncopyable {
public:
    static RoomManager& instance();

    VoiceRoomPtr createRoom(const std::string& name, UserPtr owner);
    VoiceRoomPtr findRoom(uint32_t id);
    void destroyRoom(uint32_t id);
    
    std::vector<VoiceRoomPtr> listRooms();

private:
    RoomManager() = default;
    ~RoomManager() = default;

    std::mutex mutex_;
    std::map<uint32_t, VoiceRoomPtr> rooms_;
    uint32_t next_room_id_ = 1001;
};

} // namespace lightvoice
