#include <host.hpp>
#include <enet/enet.h>
#include <log.hpp>
#include <utility.hpp>

using namespace tk::core;

namespace tk {
    namespace net {

        class Host::Impl {
        public:
            ENetHost* host;
            std::vector<ENetPeer*> peers;

            core::Event<Handle> onConnect, onDisconnect;
            core::Event<Handle, int, const Packet&> onReceive;
        };

        Host::Host() :
            impl(new Impl()),
            onConnect(impl->onConnect),
            onReceive(impl->onReceive),
            onDisconnect(impl->onDisconnect) {
            impl->host = nullptr;
        }

        Host::~Host() {
            if (impl->host) {
                enet_host_destroy(impl->host);
            }
            delete impl;
        }

        void Host::createClient(const std::string& address, int port) {
            impl->host = enet_host_create(nullptr, 32, 2, 0, 0);
            tk_assert(impl->host, "Error creating network client");

            ENetAddress remote;
            remote.port = port;
            enet_address_set_host(&remote, address.c_str());

            ENetPeer* peer = enet_host_connect(impl->host, &remote, 2, 0);
            tk_assert(peer, "Error initiating connection to server");
        }

        void Host::createServer(int port) {
            ENetAddress local;
            local.host = ENET_HOST_ANY;
            local.port = port;

            impl->host = enet_host_create(&local, 32, 2, 0, 0);
            tk_assert(impl->host, "Error create network server");
        }

        void Host::pollEvents() {
            if (impl->host == nullptr) {
                return;
            }

            ENetEvent event;
            while (enet_host_service(impl->host, &event, 0) > 0) {
                std::vector<ENetPeer*>::iterator peerIt;
                switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT:
                    impl->peers.push_back(event.peer);
                    impl->onConnect(event.peer);
                    break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    peerIt = std::find(impl->peers.begin(), impl->peers.end(), event.peer);
                    tk_assert(peerIt != impl->peers.end(), "Disconnecting an already missing peer");
                    impl->onDisconnect(event.peer);
                    impl->peers.erase(peerIt);
                    break;

                case ENET_EVENT_TYPE_RECEIVE:
                    Packet msg(event.packet->data, event.packet->data + event.packet->dataLength);
                    impl->onReceive(event.peer, event.channelID, msg);
                    enet_packet_destroy(event.packet);
                    break;
                }
            }
        }

        void Host::send(Handle handle, int channel, bool reliable, const Packet& packet) {
            ENetPacket* message = enet_packet_create(packet.data(), packet.size(), reliable ? ENET_PACKET_FLAG_RELIABLE : 0);
            enet_peer_send(reinterpret_cast<ENetPeer*>(handle), channel, message);
        }

        void Host::broadcast(int channel, bool reliable, const Packet& packet) {
            ENetPacket* message = enet_packet_create(packet.data(), packet.size(), reliable ? ENET_PACKET_FLAG_RELIABLE : 0);
            enet_host_broadcast(impl->host, channel, message);
        }

        void Host::disconnect(Handle remote) {
            enet_peer_disconnect(reinterpret_cast<ENetPeer*>(remote), 0);
            enet_host_flush(impl->host);
        }

        void Host::shutdownServer() {
            for (ENetPeer* peer : impl->peers) {
                disconnect(peer);
            }
        }
    }
}