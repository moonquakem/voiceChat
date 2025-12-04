// ====================================================================
// LightVoice: Non-copyable utility
// src/common/noncopyable.h
//
// A utility class to make derived classes non-copyable.
// Inheriting from this class deletes the copy constructor and
// copy assignment operator.
//
// Author: Gemini
// ====================================================================

#pragma once

namespace lightvoice {

class noncopyable {
public:
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;

protected:
    noncopyable() = default;
    ~noncopyable() = default;
};

} // namespace lightvoice
