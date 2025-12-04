// ====================================================================
// LightVoice: Protobuf Codec
// src/codec/ProtobufCodec.cc
//
// Implementation for the ProtobufCodec.
//
// Author: Gemini
// ====================================================================

#include "codec/ProtobufCodec.h"
#include "common/Logger.h"
#include <google/protobuf/descriptor.h>

namespace lightvoice {

void ProtobufCodec::onMessage(const net::TcpConnectionPtr& conn, net::Buffer* buf, Timestamp receiveTime) {
    while (buf->readableBytes() >= kHeaderLen) {
        const int32_t len = buf->peekInt32();
        if (len > 65536 || len < 0) {
            LOGGER_ERROR("Invalid length: {}", len);
            conn->shutdown();
            break;
        } else if (buf->readableBytes() >= kHeaderLen + len) {
            buf->retrieve(kHeaderLen);
            std::string data = buf->retrieveAsString(len);
            
            // This is a simplified parsing. A real implementation would have a
            // factory to create the correct message type based on a type name
            // also sent over the wire.
            // For this project, we might just try to parse a generic 'Packet' message.
            auto packet = std::make_shared<lightvoice::proto::Packet>();
            if(packet->ParseFromString(data)) {
                 messageCallback_(conn, packet, receiveTime);
            } else {
                LOGGER_ERROR("Failed to parse protobuf message");
            }

        } else {
            break;
        }
    }
}

void ProtobufCodec::send(const net::TcpConnectionPtr& conn, const google::protobuf::Message& message) {
    std::string data;
    message.SerializeToString(&data);
    net::Buffer buf;
    buf.append(data.c_str(), data.size());
    int32_t len = static_cast<int32_t>(data.size());
    // Manually perform host-to-big-endian conversion for portability
    int32_t be32 = 0;
    be32 |= (len & 0x000000FF) << 24;
    be32 |= (len & 0x0000FF00) << 8;
    be32 |= (len & 0x00FF0000) >> 8;
    be32 |= (len & 0xFF000000) >> 24;
    buf.prepend(&be32, sizeof(be32));
    conn->send(&buf);
}

} // namespace lightvoice
