#include <iostream>
#include <inttypes.h>
#include <arpa/inet.h>
#include <utility>
#include "serializer.h"
#include "config.h"

static std::pair<bool, std::string> read_string(TCPHandler& handler)
{
    uint8_t buffer[types::MAX_SIZE];

    std::string s;
    // Read string length.
    bool connected = handler.read_n_bytes(1, buffer);

    if (!connected) {
        return {false, nullptr};
    }

    // Read all bytes of the string.
    uint8_t len = *(uint8_t *) buffer;
    for (size_t i = 0; i < len; i++)
    {
        connected = handler.read_n_bytes(1, buffer);
        if (!connected) {
            return {false, nullptr};
        }
        char c = *(char *) buffer;
        s.append(1, c);
    }
    return {true, s};
}

static void client_check_connection(bool connected)
{
    if (!connected) {
        std::cerr << "Server disconnected!\n";
        exit(EXIT_FAILURE);
    }
}

static Hello read_hello_message(TCPHandler& handler)
{
    uint8_t buffer[types::MAX_SIZE];
    Hello message;

    auto [connected, s] = read_string(handler);
    client_check_connection(connected);
    message.server_name = s;

    connected = handler.read_n_bytes(sizeof(types::players_count_t), buffer);
    client_check_connection(connected);
    message.players_count = *(types::players_count_t *) buffer;

    connected = handler.read_n_bytes(sizeof(types::size_xy_t), buffer);
    client_check_connection(connected);
    message.size_x = *(types::size_xy_t *) buffer;

    connected = handler.read_n_bytes(sizeof(types::size_xy_t), buffer);
    client_check_connection(connected);
    message.size_y = *(types::size_xy_t *) buffer;

    connected = handler.read_n_bytes(sizeof(types::game_length_t), buffer);
    client_check_connection(connected);
    message.game_length = *(types::game_length_t *) buffer;

    connected = handler.read_n_bytes(sizeof(types::explosion_radius_t), buffer);
    client_check_connection(connected);
    message.explosion_radius = *(types::explosion_radius_t *) buffer;

    connected = handler.read_n_bytes(sizeof(types::bomb_timer_t), buffer);
    client_check_connection(connected);
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

    bool connected_to_server = handler.read_n_bytes(sizeof(types::message_id_t), buffer);
    client_check_connection(connected_to_server);

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
        std::cerr << "Unknown message received from the server!\n";
        exit(EXIT_FAILURE);
    }
}
