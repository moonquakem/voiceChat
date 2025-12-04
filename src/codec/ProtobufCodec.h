// ====================================================================
// LightVoice: Protobuf Codec
// src/codec/ProtobufCodec.h
//
// A codec for handling Protobuf messages over a TCP stream.
// It handles the length-prefix framing protocol.
//
// Author: Gemini
// ====================================================================

#pragma once

#include "net/TcpConnection.h"
#include "net/Buffer.h"
#include <google/protobuf/message.h>

namespace lightvoice {

using MessagePtr = std::shared_ptr<google::protobuf::Message>;

class ProtobufCodec {
public:
    using ProtobufMessageCallback = std::function<void(const net::TcpConnectionPtr&, const MessagePtr&, Timestamp)>;

    explicit ProtobufCodec(ProtobufMessageCallback cb)
        : messageCallback_(std::move(cb)) {}

    void onMessage(const net::TcpConnectionPtr& conn, net::Buffer* buf, Timestamp receiveTime);
    void send(const net::TcpConnectionPtr& conn, const google::protobuf::Message& message);

private:
    const static int kHeaderLen = sizeof(int32_t);
    ProtobufMessageCallback messageCallback_;
};

} // namespace lightvoice
