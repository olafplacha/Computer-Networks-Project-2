#include <iostream>
#include <inttypes.h>
#include <arpa/inet.h>
#include "serializer.h"
#include "config.h"

static std::string read_string(TCPHandler& handler)
{
    uint8_t buffer[types::MAX_SIZE];

    std::string s;

    // Read string length.
    handler.read_n_bytes(1, buffer);

    // Read all bytes of the string.
    uint8_t len = *(uint8_t *) buffer;
    for (size_t i = 0; i < len; i++)
    {
        handler.read_n_bytes(1, buffer);
        char c = *(char *) buffer;
        s.append(1, c);
    }
    return s;
}

static Hello read_hello_message(TCPHandler& handler)
{
    uint8_t buffer[types::MAX_SIZE];
    Hello message;

    std::string s = read_string(handler);
    message.server_name = s;

    handler.read_n_bytes(sizeof(types::players_count_t), buffer);
    message.players_count = *(types::players_count_t *) buffer;

    handler.read_n_bytes(sizeof(types::size_xy_t), buffer);
    message.size_x = *(types::size_xy_t *) buffer;

    handler.read_n_bytes(sizeof(types::size_xy_t), buffer);
    message.size_y = *(types::size_xy_t *) buffer;

    handler.read_n_bytes(sizeof(types::game_length_t), buffer);
    message.game_length = *(types::game_length_t *) buffer;

    handler.read_n_bytes(sizeof(types::explosion_radius_t), buffer);
    message.explosion_radius = *(types::explosion_radius_t *) buffer;

    handler.read_n_bytes(sizeof(types::bomb_timer_t), buffer);
    message.bomber_timer = *(types::bomb_timer_t *) buffer;

    return message;
}

static AcceptedPlayer read_accepted_player_message(TCPHandler& handler)
{
    uint8_t buffer[types::MAX_SIZE];
    AcceptedPlayer message;

    // TODO.
    return message;
}

static GameStarted read_game_started_message(TCPHandler& handler)
{
    uint8_t buffer[types::MAX_SIZE];
    GameStarted message;

    // TODO.
    return message;
}

static Turn read_turn_message(TCPHandler& handler)
{
    uint8_t buffer[types::MAX_SIZE];
    Turn message;

    // TODO.
    return message;
}

static GameEnded read_game_ended_message(TCPHandler& handler)
{
    uint8_t buffer[types::MAX_SIZE];
    GameEnded message;

    // TODO.
    return message;
}

ServerMessage read_client_server_message(TCPHandler& handler)
{
    uint8_t buffer[types::MAX_SIZE];

    handler.read_n_bytes(sizeof(types::message_id_t), buffer);
    types::message_id_t message_id = *(types::message_id_t *) buffer;

    switch (message_id)
    {
    case 0:
        return read_hello_message(handler);
    case 1:
        return read_accepted_player_message(handler);
    case 2:
        return read_game_started_message(handler);
    case 3:
        return read_turn_message(handler);
    case 4:
        return read_game_ended_message(handler);
    default:
        throw std::runtime_error("Unknown message received from the server!");
    }
}
