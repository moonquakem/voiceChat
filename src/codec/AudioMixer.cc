// ====================================================================
// LightVoice: Audio Mixer
// src/codec/AudioMixer.cc
//
// Implementation of the AudioMixer class.
//
// Author: Gemini
// ====================================================================

#include "codec/AudioMixer.h"
#include "codec/OpusDecoder.h"
#include "codec/OpusEncoder.h"
#include "common/Logger.h"
#include <algorithm>
#include <numeric>

namespace lightvoice {

AudioMixer::AudioMixer(opus_int32 sample_rate, int channels, int frame_size)
    : sample_rate_(sample_rate),
      channels_(channels),
      frame_size_(frame_size),
      decoder_(std::make_unique<OpusDecoder>(sample_rate, channels)),
      encoder_(std::make_unique<OpusEncoder>(sample_rate, channels, frame_size)) {
    
    mix_buffer_.resize(frame_size_ * channels_);
}

AudioMixer::~AudioMixer() = default;

AudioFramePtr AudioMixer::mix(const std::vector<AudioFramePtr>& frames) {
    if (frames.empty()) {
        return nullptr;
    }

    std::fill(mix_buffer_.begin(), mix_buffer_.end(), 0);
    std::vector<std::vector<int16_t>> decoded_pcms;
    decoded_pcms.reserve(frames.size());

    // 1. Decode all frames
    for (const auto& frame : frames) {
        std::vector<int16_t> pcm_buffer;
        int decoded_samples = decoder_->decode(*frame, pcm_buffer, frame_size_);
        if (decoded_samples == frame_size_) {
            decoded_pcms.push_back(std::move(pcm_buffer));
        }
    }
    
    if (decoded_pcms.empty()) {
        return nullptr;
    }

    // 2. Mix (Additive mixing)
    for (const auto& pcm_buffer : decoded_pcms) {
        for (size_t i = 0; i < mix_buffer_.size(); ++i) {
            // Use 32-bit integers for intermediate summation to prevent overflow
            int32_t sample = static_cast<int32_t>(mix_buffer_[i]) + pcm_buffer[i];
            // Clamp to 16-bit range (hard clipping)
            mix_buffer_[i] = std::clamp(sample, -32768, 32767);
        }
    }
    
    // 3. Soft clipping (optional, simple division if too loud)
    // A more sophisticated soft clipper would use a curve (e.g., tanh).
    if (decoded_pcms.size() > 2) {
        for (size_t i = 0; i < mix_buffer_.size(); ++i) {
            mix_buffer_[i] /= (decoded_pcms.size() / 2);
        }
    }


    // 4. Re-encode the mixed buffer
    encoder_->encode(mix_buffer_, mixed_opus_buffer_);
    
    if (mixed_opus_buffer_.empty()) {
        return nullptr;
    }

    return std::make_shared<AudioFrame>(mixed_opus_buffer_);
}

} // namespace lightvoice
