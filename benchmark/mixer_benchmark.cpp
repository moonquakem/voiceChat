// ====================================================================
// LightVoice: Mixer Benchmark
// benchmark/mixer_benchmark.cpp
//
// A benchmark to test the performance of the AudioMixer.
//
// Author: Gemini
// ====================================================================

#include "codec/AudioMixer.h"
#include "codec/OpusEncoder.h"
#include "common/Logger.h"
#include <chrono>
#include <numeric>

using namespace lightvoice;

// Create a dummy silent Opus frame
AudioFramePtr create_silent_frame(OpusEncoder& encoder) {
    std::vector<int16_t> pcm(960, 0);
    auto frame = std::make_shared<AudioFrame>();
    encoder.encode(pcm, *frame);
    return frame;
}

int main() {
    Logger::Init();

    const int sample_rate = 48000;
    const int channels = 1;
    const int frame_size = 960; // 20ms

    AudioMixer mixer(sample_rate, channels, frame_size);
    OpusEncoder encoder(sample_rate, channels, frame_size);
    
    auto silent_frame = create_silent_frame(encoder);

    const int num_speakers[] = {2, 4, 8, 16, 32};
    const int iterations = 1000;

    LOGGER_INFO("--- AudioMixer Benchmark ---");
    LOGGER_INFO("Sample Rate: {}, Frame Size: {}", sample_rate, frame_size);
    LOGGER_INFO("Iterations per test: {}", iterations);

    for (int speakers : num_speakers) {
        std::vector<AudioFramePtr> frames(speakers, silent_frame);
        
        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < iterations; ++i) {
            AudioFramePtr mixed_frame = mixer.mix(frames);
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        
        double avg_time = duration.count() / iterations;
        LOGGER_INFO("Speakers: {:<4} | Avg time per mix: {:<8.4f} ms", speakers, avg_time);
    }
    
    return 0;
}
