#pragma once

#include <event.hpp>
#include <host.hpp>
#include <serialize.hpp>
#include <messages.hpp>
#include <player_table.hpp>
#include <map>
#include <string>

namespace tk {
    namespace net {

        template <class PlayerInfo>
        class Client {
            Host host;
            Host::Handle server;
            PlayerInfo localPlayer;
            PlayerTable<PlayerInfo> players;
            int localID;

            core::Delegate<Host::Handle> onConnect, onDisconnect;
            core::Delegate<Host::Handle, int, const Host::Packet&> onReceive;

            void handlePlayerID(Host::Packet::const_iterator msg) {
                core::deserialize(msg, localID);
                tk_info(core::format("Client received player ID %%", localID));
                host.send(server, 0, true, (MessageType)Message::PlayerData, localPlayer);
            }

            void handleNewPlayer(Host::Packet::const_iterator msg) {
                core::deserialize(msg, players);
            }

        public:
            core::Event<> onPlayerConnected;
            core::Event<> onPlayerDisconnected;
            core::Event<> onPlayerTimeout;
            core::Event<> onMessageReceived;

            core::Event<> onConnectedToServer;
            core::Event<> onServerTimeout;

            Client(PlayerInfo local) : localPlayer(local), localID(-1) { 
                onConnect.event = [this] (Host::Handle connection) {
                    server = connection;
                };

                onDisconnect.event = [this] (Host::Handle connection) {
                };

                onReceive.event = [this] (Host::Handle connection, int channel, const Host::Packet& packet) {
                    if (channel == 0) {
                        Host::Packet::const_iterator it = packet.begin();
                        MessageType type;
                        core::deserialize(it, type);

                        tk_info(core::format("New message: %%", (int)type));

                        switch (type) {
                        case Message::PlayerID:
                            handlePlayerID(it);
                            break;
                        case Message::NewPlayer:
                            handleNewPlayer(it);
                            break;
                        }
                    } else {
                        // client message
                    }
                };

                host.onConnect.attach(onConnect);
                host.onDisconnect.attach(onDisconnect);
                host.onReceive.attach(onReceive);
            }

            ~Client() { }

            void connect(const std::string& address, int port) {
                host.createClient(address, port);
            }

            void pollEvents() {
                host.pollEvents();
            }

        };
    }
}