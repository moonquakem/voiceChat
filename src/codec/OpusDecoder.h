// ====================================================================
// LightVoice: Opus Decoder
// src/codec/OpusDecoder.h
//
// A C++ wrapper around the libopus decoder. This class is
// responsible for taking Opus packets and decoding them back into
// raw PCM audio data.
//
// Author: Gemini
// ====================================================================

#pragma once

#include "common/noncopyable.h"
#include <opus/opus.h>
#include <vector>
#include <cstdint>

namespace lightvoice {

class OpusDecoder : noncopyable {
public:
    OpusDecoder(opus_int32 sample_rate, int channels);
    ~OpusDecoder();

    // Decodes an Opus packet into a PCM audio frame.
    // opus_data: Input buffer of Opus data.
    // pcm: Output buffer for the decoded int16_t samples.
    // frame_size: The desired number of samples per channel in the output.
    // Returns the number of samples decoded per channel, or a negative error code.
    int decode(const std::vector<unsigned char>& opus_data, std::vector<int16_t>& pcm, int frame_size);

private:
    OpusDecoder* decoder_ = nullptr;
    int channels_;
};

} // namespace lightvoice
