// ====================================================================
// LightVoice: Voice Room
// src/room/VoiceRoom.cc
//
// Implementation for the VoiceRoom class.
//
// Author: Gemini
// ====================================================================

#include "room/VoiceRoom.h"
#include "room/User.h" // Assuming User class exists
#include "net/TcpConnection.h"
#include "common/Logger.h"
#include "proto/chat.pb.h"
#include "codec/ProtobufCodec.h" // Assuming this exists

namespace lightvoice {

// Assume a global ProtobufCodec instance for sending messages
extern ProtobufCodec* g_codec;

VoiceRoom::VoiceRoom(uint32_t id, std::string name, UserPtr owner)
    : id_(id),
      name_(std::move(name)),
      owner_(owner),
      mixer_(std::make_unique<AudioMixer>(48000, 1, 960)) {
    LOGGER_INFO("VoiceRoom created: {} ({})", name_, id_);
}

VoiceRoom::~VoiceRoom() {
    LOGGER_INFO("VoiceRoom destroyed: {} ({})", name_, id_);
}

void VoiceRoom::start() {
    // This is where we would start the 20ms mix timer
    // For simplicity, we'll simulate it. In a real app, this would be
    // driven by the TimerManager in the Mixer thread.
}

void VoiceRoom::stop() {
    // Stop the mix timer
}

void VoiceRoom::addUser(UserPtr user) {
    std::lock_guard<std::mutex> lock(mutex_);
    members_[user->id()] = user;
    user->setRoom(shared_from_this());

    // Notify others
    proto::RoomNotification notif;
    notif.set_type(proto::RoomNotification::JOIN);
    notif.set_user_id(user->id());
    notif.set_username(user->name());
    notif.set_message(user->name() + " has joined the room.");
    broadcastMessage(notif);

    LOGGER_INFO("User {} joined room {}", user->name(), name_);
}

void VoiceRoom::removeUser(UserPtr user) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        members_.erase(user->id());
        user->clearRoom();
    }
    
    // Notify others
    proto::RoomNotification notif;
    notif.set_type(proto::RoomNotification::LEAVE);
    notif.set_user_id(user->id());
    notif.set_username(user->name());
    notif.set_message(user->name() + " has left the room.");
    broadcastMessage(notif);
    
    LOGGER_INFO("User {} left room {}", user->name(), name_);
}

void VoiceRoom::onAudioPacket(uint32_t userId, AudioFramePtr frame) {
    // This function would be called by the IO thread.
    // It should push the frame to a lock-free queue for the mixer thread.
    std::lock_guard<std::mutex> lock(mutex_);
    pending_frames_.push_back(frame);
}

void VoiceRoom::onMixTimer() {
    // This function is called by the Mixer thread every 20ms.
    std::vector<AudioFramePtr> frames_to_mix;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (pending_frames_.empty()) {
            return;
        }
        frames_to_mix.swap(pending_frames_);
    }

    AudioFramePtr mixed_frame = mixer_->mix(frames_to_mix);
    if (!mixed_frame) {
        return;
    }

    // Broadcast mixed audio to all members
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& pair : members_) {
        // This is a simplification. In reality, you'd have a custom voice protocol.
        // You would not use the protobuf codec for high-frequency voice data.
        // We'll just log it for now.
        // pair.second->conn()->send(*mixed_frame);
    }
    LOGGER_TRACE("Mixed {} frames, sent {} bytes to {} members", frames_to_mix.size(), mixed_frame->size(), members_.size());
}

void VoiceRoom::broadcastMessage(const google::protobuf::Message& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    for(const auto& pair : members_) {
        if (g_codec) {
            // g_codec->send(pair.second->conn(), message);
        }
    }
}


} // namespace lightvoice
