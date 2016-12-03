#pragma once

#include <event.hpp>
#include <host.hpp>
#include <serialize.hpp>
#include <messages.hpp>
#include <utility.hpp>
#include <log.hpp>
#include <player_table.hpp>
#include <map>
#include <string>
#include <algorithm>


namespace tk {
    namespace net {

        template <class PlayerInfo>
        class Server {
            Host host;

            int freePlayerID;
            PlayerTable<PlayerInfo> players;

            core::Delegate<Host::Handle> onConnect, onDisconnect;
            core::Delegate<Host::Handle, int, const Host::Packet&> onReceive;

            void handlePlayerData(Host::Handle handle, Host::Packet::const_iterator msg) {
                typename PlayerTable<PlayerInfo>::Player* player = players.get(handle);
                core::deserialize(msg, player->info);
                host.broadcast(0, true, (MessageType)Message::NewPlayer, players);

                onPlayerConnected(player->id);
            }

        public:
            core::Event<int> onPlayerConnected;
            core::Event<int> onPlayerDisconnected;
            core::Event<> onPlayerTimeout;
            core::Event<> onMessageReceived;

            Server(PlayerInfo local) : freePlayerID(2) {
                players.add(1, nullptr, local);

                onConnect.event = [this] (Host::Handle connection) {
                    int newID = freePlayerID++;
                    tk_info(format("New player connected (%%)", newID));
                    players.add(newID, connection);

                    host.send(connection, 0, true, (MessageType)Message::PlayerID, newID);
                };

                onDisconnect.event = [this] (Host::Handle connection) {
                    auto player = players.get(connection);
                    onPlayerDisconnected(player->id);
                    players.remove(player->id);
                };

                onReceive.event = [this] (Host::Handle connection, int channel, const Host::Packet& packet) {
                    if (channel == 0) {
                        Host::Packet::const_iterator it = packet.begin();
                        MessageType type;
                        core::deserialize(it, type);

                        switch (type) {
                        case Message::PlayerData:
                            handlePlayerData(connection, it);
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

            ~Server() { }

            void start(int port) {
                host.createServer(port);
            }

            void pollEvents() {
                host.pollEvents();
            }


        };
    }
}