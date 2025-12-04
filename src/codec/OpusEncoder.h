// ====================================================================
// LightVoice: Opus Encoder
// src/codec/OpusEncoder.h
//
// A C++ wrapper around the libopus encoder. This class is
// responsible for taking raw PCM audio data and encoding it into
// Opus packets.
//
// Author: Gemini
// ====================================================================

#pragma once

#include "common/noncopyable.h"
#include <opus/opus.h>
#include <vector>
#include <cstdint>

namespace lightvoice {

class OpusEncoder : noncopyable {
public:
    // sample_rate: e.g., 48000
    // channels: 1 (mono) or 2 (stereo)
    // frame_size: e.g., 960 for 20ms at 48kHz
    OpusEncoder(opus_int32 sample_rate, int channels, int frame_size);
    ~OpusEncoder();

    // Encodes a single frame of PCM data.
    // pcm: Input buffer of int16_t samples.
    // output: Buffer to store the encoded Opus data.
    // Returns the number of bytes written to the output buffer.
    int encode(const std::vector<int16_t>& pcm, std::vector<unsigned char>& output);

private:
    OpusEncoder* encoder_ = nullptr;
    int channels_;
    int frame_size_;
};

} // namespace lightvoice
