// ====================================================================
// LightVoice: Opus Decoder
// src/codec/OpusDecoder.cc
//
// Implementation of the OpusDecoder class.
//
// Author: Gemini
// ====================================================================

#include "codec/OpusDecoder.h"
#include "common/Logger.h"

namespace lightvoice {

OpusDecoder::OpusDecoder(opus_int32 sample_rate, int channels)
    : channels_(channels) {
    int error;
    decoder_ = opus_decoder_create(sample_rate, channels, &error);
    if (error != OPUS_OK) {
        LOGGER_CRITICAL("Failed to create Opus decoder: {}", opus_strerror(error));
    }
}

OpusDecoder::~OpusDecoder() {
    if (decoder_) {
        opus_decoder_destroy(decoder_);
    }
}

int OpusDecoder::decode(const std::vector<unsigned char>& opus_data, std::vector<int16_t>& pcm, int frame_size) {
    pcm.resize(frame_size * channels_);

    int decoded_samples = opus_decode(decoder_, opus_data.empty() ? nullptr : opus_data.data(), opus_data.size(), pcm.data(), frame_size, 0);

    if (decoded_samples < 0) {
        LOGGER_ERROR("Opus decoding failed: {}", opus_strerror(decoded_samples));
        return decoded_samples;
    }
    
    pcm.resize(decoded_samples * channels_);
    return decoded_samples;
}

} // namespace lightvoice
