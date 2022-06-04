#include "lobby.h"

LobbyClient::LobbyClient(const Hello& hello)
{
    server_name = hello.server_name;
    players_count = hello.players_count;
    size_x = hello.size_x;
    size_y = hello.size_y;
    game_length = hello.game_length;
    explosion_radius = hello.explosion_radius;
    bomb_timer = hello.bomb_timer;
}

void LobbyClient::accept(const AcceptedPlayer& player)
{
    players.insert({player.id, player.player});
}

LobbyMessage LobbyClient::get_lobby_state() const
{
    LobbyMessage message;
    message.server_name = server_name;
    message.players_count = players_count;
    message.size_x = size_x;
    message.size_y = size_y;
    message.game_length = game_length;
    message.explosion_radius = explosion_radius;
    message.bomb_timer = bomb_timer;
    message.players = players;

    return message;
}