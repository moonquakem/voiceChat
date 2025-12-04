// ====================================================================
// LightVoice: Room Manager
// src/room/RoomManager.cc
//
// Implementation for the RoomManager class.
//
// Author: Gemini
// ====================================================================

#include "room/RoomManager.h"

namespace lightvoice {

RoomManager& RoomManager::instance() {
    static RoomManager instance;
    return instance;
}

VoiceRoomPtr RoomManager::createRoom(const std::string& name, UserPtr owner) {
    std::lock_guard<std::mutex> lock(mutex_);
    uint32_t id = next_room_id_++;
    auto room = std::make_shared<VoiceRoom>(id, name, owner);
    rooms_[id] = room;
    return room;
}

VoiceRoomPtr RoomManager::findRoom(uint32_t id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = rooms_.find(id);
    if (it != rooms_.end()) {
        return it->second;
    }
    return nullptr;
}

void RoomManager::destroyRoom(uint32_t id) {
    std::lock_guard<std::mutex> lock(mutex_);
    rooms_.erase(id);
}

std::vector<VoiceRoomPtr> RoomManager::listRooms() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<VoiceRoomPtr> list;
    list.reserve(rooms_.size());
    for(const auto& pair : rooms_) {
        list.push_back(pair.second);
    }
    return list;
}

} // namespace lightvoice
