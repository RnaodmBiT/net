#pragma once
#include <cstdint>

namespace tk {
    namespace net {

        typedef uint8_t MessageType;

        enum Message : MessageType {
            PlayerID,       // Server -> Client
            PlayerData,     // Client -> Server
            NewPlayer,      // Broadcast
            Count
        };

    }
}