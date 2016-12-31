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
                host.broadcast(0, true, (MessageType)Message::PlayerConnected, player->id, players);
                onPlayerConnected(player->id);
            }

        public:
            core::Event<int> onPlayerConnected;
            core::Event<int> onPlayerDisconnected;
            core::Event<int, const Host::Packet&> onMessageReceived;

            Server() : freePlayerID(1) {
                onConnect.event = [this] (Host::Handle connection) {
                    int newID = freePlayerID++;
                    tk_info(core::format("New player connected (%%)", newID));
                    players.add(newID, connection);
                    host.send(connection, 0, true, (MessageType)Message::PlayerID, newID);
                };

                onDisconnect.event = [this] (Host::Handle connection) {
                    int id = players.get(connection)->id;
                    onPlayerDisconnected(id);
                    players.remove(id);
                    host.broadcast(0, true, (MessageType)Message::PlayerDisconnected, id, players);
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
                        onMessageReceived(players.get(connection)->id, packet);
                    }
                };

                host.onConnect.attach(onConnect);
                host.onDisconnect.attach(onDisconnect);
                host.onReceive.attach(onReceive);
            }

            ~Server() { }

            PlayerInfo* getPlayer(int id) {
                typename PlayerTable<PlayerInfo>::Player* player = players.get(id);
                return player ? &player->info : nullptr;
            }

            std::vector<int> getPlayerIDs() const {
                std::vector<int> ids;
                for (auto& p : players.playerList) {
                    ids.push_back(p.id);
                }
                return ids;
            }

            void updatePlayerTable() {
                host.broadcast(0, true, (MessageType)Message::PlayerTableUpdate, players);
            }

            void start(int port) {
                host.createServer(port);
            }

            void pollEvents() {
                host.pollEvents();
            }

            void disconnect() {
                host.shutdownServer();
            }

            template <class ...Args>
            void broadcast(bool reliable, const Args&... args) {
                host.broadcast(1, reliable, args...);
            }

            void broadcast(bool reliable, const Host::Packet& packet) {
                host.broadcast(1, reliable, packet);
            }

            template <class ...Args>
            void send(int player, bool reliable, const Args&... args) {
                host.send(players.get(player)->handle, 1, reliable, args...);
            }

            void send(int player, bool reliable, const Host::Packet& packet) {
                host.send(players.get(player)->handle, 1, reliable, packet);
            }
        };
    }
}