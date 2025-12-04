// ====================================================================
// LightVoice: Opus Encoder
// src/codec/OpusEncoder.cc
//
// Implementation of the OpusEncoder class.
//
// Author: Gemini
// ====================================================================

#include "codec/OpusEncoder.h"
#include "common/Logger.h"

namespace lightvoice {

OpusEncoder::OpusEncoder(opus_int32 sample_rate, int channels, int frame_size)
    : channels_(channels), frame_size_(frame_size) {
    int error;
    encoder_ = opus_encoder_create(sample_rate, channels, OPUS_APPLICATION_VOIP, &error);
    if (error != OPUS_OK) {
        LOGGER_CRITICAL("Failed to create Opus encoder: {}", opus_strerror(error));
    }
    // Set a reasonable bitrate
    opus_encoder_ctl(encoder_, OPUS_SET_BITRATE(64000));
}

OpusEncoder::~OpusEncoder() {
    if (encoder_) {
        opus_encoder_destroy(encoder_);
    }
}

int OpusEncoder::encode(const std::vector<int16_t>& pcm, std::vector<unsigned char>& output) {
    if (pcm.size() != static_cast<size_t>(frame_size_ * channels_)) {
        LOGGER_ERROR("OpusEncoder: incorrect PCM size. Expected {}, got {}", frame_size_ * channels_, pcm.size());
        return 0;
    }
    
    // Opus recommends a max packet size for safety
    output.resize(4000);

    int encoded_bytes = opus_encode(encoder_, pcm.data(), frame_size_, output.data(), output.size());

    if (encoded_bytes < 0) {
        LOGGER_ERROR("Opus encoding failed: {}", opus_strerror(encoded_bytes));
        return 0;
    }
    
    output.resize(encoded_bytes);
    return encoded_bytes;
}

} // namespace lightvoice
