#pragma once
#include <host.hpp>
#include <core.hpp>
#include <vector>

namespace tk {
    namespace net {

        template <class PlayerInfo>
        class PlayerTable {
        public:
            struct Player {
                int id;
                Host::Handle handle;
                PlayerInfo info;
            };

            void add(int id, Host::Handle handle, PlayerInfo info = PlayerInfo()) {
                playerList.push_back({ id, handle, info });
            }

            Player* get(int id, bool assert = false) {
                auto it = std::find_if(playerList.begin(), playerList.end(), [id] (const Player& p) {
                    return p.id == id;
                });

                if (it == playerList.end()) {
                    tk_assert(!assert, core::format("Could not find player with id %%", id));
                    return nullptr;
                } else {
                    return &(*it);
                }
            }

            Player* get(Host::Handle handle) {
                auto it = std::find_if(playerList.begin(), playerList.end(), [handle] (const Player& p) {
                    return p.handle == handle;
                });

                tk_assert(it != playerList.end(), "Could not find player using connection handle");
                return &(*it);
            }

            void remove(int id) {
                auto it = std::find_if(playerList.begin(), playerList.end(), [id] (const Player& p) {
                    return p.id == id;
                });

                tk_assert(it != playerList.end(), core::format("Player %% could not be removed. Was not in the player table", id));
                playerList.erase(it);
            }

            std::vector<int> getIDs() const {
                std::vector<int> ids;
                for (const Player& player : playerList) {
                    ids.push_back(player.id);
                }
                return ids;
            }

            std::vector<Player> playerList;
        };

    }

    namespace core {
        template <class T>
        struct convert<net::PlayerTable<T>> {
            void serialize(Blob& blob, const net::PlayerTable<T>& table) {
                tk::core::serialize(blob, (int)table.playerList.size());
                for (int i = 0; i < table.playerList.size(); ++i) {
                    tk::core::serialize(blob, table.playerList[i].id, table.playerList[i].info);
                }
            }

            void deserialize(Blob::const_iterator& it, net::PlayerTable<T>& table) {
                int size;
                tk::core::deserialize(it, size);
                table.playerList.clear();
                table.playerList.resize(size);
                for (int i = 0; i < size; ++i) {
                    table.playerList[i].handle = nullptr;
                    tk::core::deserialize(it, table.playerList[i].id, table.playerList[i].info);
                }
            }
        };
    }
}
