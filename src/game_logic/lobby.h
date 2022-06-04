#ifndef LOBBY_H
#define LOBBY_H

#include "../network/messages.h"

class Lobby {
protected:
    std::string server_name;
    types::players_count_t players_count;
    types::size_xy_t size_x;
    types::size_xy_t size_y;
    types::game_length_t game_length;
    types::explosion_radius_t explosion_radius;
    types::bomb_timer_t bomb_timer;
    std::map<types::player_id_t, Player> players;
};

class LobbyClient : public Lobby {
public:
    /* Instantiate Lobby based on the first message from the server. */
    explicit LobbyClient(const Hello &);

    /* Change Lobby's state when a new player is accepted */
    void accept(const AcceptedPlayer &);

    /* Get the state of the lobby. */
    [[nodiscard]] LobbyMessage get_lobby_state() const;
};

#endif // LOBBY_H