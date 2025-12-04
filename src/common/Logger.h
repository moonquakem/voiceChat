// ====================================================================
// LightVoice: Logger utility
// src/common/Logger.h
//
// A simple singleton logger wrapper around the spdlog library.
// Provides easy access to a globally configured logger.
//
// Author: Gemini
// ====================================================================

#pragma once

// Define this to use the compiled spdlog library, not the header-only version.
// This must come before any spdlog includes.
#define SPDLOG_COMPILED_LIB

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "fmt/core.h"
#include <memory>

namespace lightvoice {

class Logger {
public:
    static void Init() {
        if (!logger_) {
            // Create a color multi-threaded console logger
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [thread %t] %v");
            
            logger_ = std::make_shared<spdlog::logger>("lightvoice", console_sink);
            spdlog::register_logger(logger_);
            logger_->set_level(spdlog::level::trace); // Set default level
            logger_->flush_on(spdlog::level::trace);
        }
    }

    static std::shared_ptr<spdlog::logger>& GetLogger() {
        if (!logger_) {
            Init();
        }
        return logger_;
    }

private:
    static std::shared_ptr<spdlog::logger> logger_;
};

// Initialize static member
inline std::shared_ptr<spdlog::logger> Logger::logger_ = nullptr;

} // namespace lightvoice

// --- Logger Macros ---
// Use these macros for convenient logging throughout the application.

#define LOGGER_TRACE(...) ::lightvoice::Logger::GetLogger()->trace(__VA_ARGS__)
#define LOGGER_DEBUG(...) ::lightvoice::Logger::GetLogger()->debug(__VA_ARGS__)
#define LOGGER_INFO(...)  ::lightvoice::Logger::GetLogger()->info(__VA_ARGS__)
#define LOGGER_WARN(...)  ::lightvoice::Logger::GetLogger()->warn(__VA_ARGS__)
#define LOGGER_ERROR(...) ::lightvoice::Logger::GetLogger()->error(__VA_ARGS__)
#define LOGGER_CRITICAL(...) ::lightvoice::Logger::GetLogger()->critical(__VA_ARGS__)

