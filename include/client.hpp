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

            bool hasCompletedConnection;

            core::Delegate<Host::Handle> onConnect, onDisconnect;
            core::Delegate<Host::Handle, int, const Host::Packet&> onReceive;

            void handlePlayerID(Host::Packet::const_iterator msg) {
                core::deserialize(msg, id);
                tk_info(core::format("Client received player ID %%", id));
                host.send(server, 0, true, (MessageType)Message::PlayerData, localPlayer);
            }

            void handlePlayerConnected(Host::Packet::const_iterator msg) {
                int newPlayer;
                core::deserialize(msg, newPlayer, players);
                tk_info(core::format("Client received new connection for %%", newPlayer));
                if (!hasCompletedConnection && newPlayer == id) {
                    hasCompletedConnection = true;
                    onConnectedToServer();
                } else {
                    onPlayerConnected(newPlayer);
                }
            }

            void handlePlayerDisconnected(Host::Packet::const_iterator msg) {
                int deadPlayer;
                core::deserialize(msg, deadPlayer);
                tk_info(core::format("Client received disconnection for %%", deadPlayer));
                onPlayerDisconnected(deadPlayer);
                core::deserialize(msg, players);
            }

            void handlePlayerTableUpdate(Host::Packet::const_iterator msg) {
                tk_info("Received updated player table");
                core::deserialize(msg, players);
            }

        public:
            core::Event<int> onPlayerConnected;
            core::Event<int> onPlayerDisconnected;
            core::Event<const Host::Packet&> onMessageReceived;

            core::Event<> onConnectedToServer;
            core::Event<> onServerDisconnected;

            PlayerTable<PlayerInfo> players;
            int id;

            Client() : id(-1), hasCompletedConnection(false) {
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

                        switch (type) {
                        case Message::PlayerID:
                            handlePlayerID(it);
                            break;
                        case Message::PlayerConnected:
                            handlePlayerConnected(it);
                            break;
                        case Message::PlayerDisconnected:
                            handlePlayerDisconnected(it);
                            break;
                        case Message::PlayerTableUpdate:
                            handlePlayerTableUpdate(it);
                            break;
                        }
                    } else {
                        onMessageReceived(packet);
                    }
                };

                host.onConnect.attach(onConnect);
                host.onDisconnect.attach(onDisconnect);
                host.onReceive.attach(onReceive);
            }

            ~Client() { }

            PlayerInfo* getPlayer(int pid = -1) {
                if (pid == -1) {
                    pid = id;
                }
                typename PlayerTable<PlayerInfo>::Player* player = players.get(pid);
                return player ? &player->info : nullptr;
            }

            std::vector<int> getPlayerIDs() const {
                std::vector<int> ids;
                for (auto& p : players.playerList) {
                    ids.push_back(p.id);
                }
                return ids;
            }

            void connect(const std::string& address, int port, PlayerInfo local) {
                localPlayer = local;
                host.createClient(address, port);
            }

            void pollEvents() {
                host.pollEvents();
            }

            void disconnect() {
                host.disconnect(server);
            }

            template <class ...Args>
            void send(bool reliable, const Args&... args) {
                host.send(server, 1, true, args...);
            }

        };
    }
}