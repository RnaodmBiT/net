#pragma once
#include <linkage.hpp>
#include <event.hpp>
#include <serialize.hpp>
#include <string>
#include <vector>
#include <cstdint>

namespace tk {
    namespace net {
        class TK_NET Host {
            class Impl;
            Impl* impl;

        public:

            typedef void* Handle;
            typedef std::vector<uint8_t> Packet;

        private:

            void send(Handle handle, int channel, bool reliable, const Packet& packet);
            void broadcast(int channel, bool reliable, const Packet& packet);

        public:

            core::Event<Handle>& onConnect;
            core::Event<Handle, int, const Packet&>& onReceive;
            core::Event<Handle>& onDisconnect;

            Host();
            ~Host();

            Host(Host&& move) = delete;
            Host(const Host& copy) = delete;

            void createClient(const std::string& address, int port);
            void createServer(int port);

            void pollEvents();

            template <class ...Args>
            void send(Handle handle, int channel, bool reliable, const Args&... args) {
                Packet message;
                core::serialize(message, args...);
                send(handle, channel, reliable, message);
            }

            template <class ...Args>
            void broadcast(int channel, bool reliable, const Args&... args) {
                Packet message;
                core::serialize(message, args...);
                broadcast(channel, reliable, message);
            }
        };
    }
}