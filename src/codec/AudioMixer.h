// ====================================================================
// LightVoice: Audio Mixer
// src/codec/AudioMixer.h
//
// Responsible for mixing audio from multiple sources within a single
// voice room. It decodes incoming Opus packets, mixes the raw PCM
// audio, applies a soft clipping algorithm to prevent distortion,
// and then re-encodes the mixed audio back into a single Opus packet.
//
// Author: Gemini
// ====================================================================

#pragma once

#include "common/noncopyable.h"
#include <vector>
#include <cstdint>
#include <memory>

namespace lightvoice {

class OpusDecoder;
class OpusEncoder;

using AudioFrame = std::vector<unsigned char>;
using AudioFramePtr = std::shared_ptr<AudioFrame>;

class AudioMixer : noncopyable {
public:
    AudioMixer(opus_int32 sample_rate, int channels, int frame_size);
    ~AudioMixer();

    // Mixes a collection of Opus frames.
    // frames: A vector of shared_ptr to Opus audio frames.
    // Returns a single mixed Opus frame.
    AudioFramePtr mix(const std::vector<AudioFramePtr>& frames);

private:
    opus_int32 sample_rate_;
    int channels_;
    int frame_size_; // e.g., 960 for 20ms at 48kHz

    std::unique_ptr<OpusDecoder> decoder_;
    std::unique_ptr<OpusEncoder> encoder_;

    // Pre-allocated buffers for performance
    std::vector<int16_t> mix_buffer_;
    std::vector<unsigned char> mixed_opus_buffer_;
};

} // namespace lightvoice
