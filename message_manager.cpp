#include "message_manager.h"

ClientMessageManager::ClientMessageManager(TCPHandler& tcp_handler_, UDPHandler& udp_handler_) :
    tcp_handler(tcp_handler_), udp_handler(udp_handler_) {}

ServerMessage ClientMessageManager::read_server_message()
{
    types::message_id_t message_id = tcp_handler.read_element<types::message_id_t>();

    switch (message_id)
    {
    case clientServerCodes::hello:
        return Hello(tcp_handler);

    case clientServerCodes::acceptedPlayer:
        return AcceptedPlayer(tcp_handler);

    case clientServerCodes::gameStarted:
        return GameStarted(tcp_handler);

    case clientServerCodes::turn:
        return Turn(tcp_handler);

    case clientServerCodes::gameEnded:
        return GameEnded(tcp_handler);

    default:
        throw std::runtime_error("Unknown message received from the server!");
    }
}

InputMessage ClientMessageManager::read_gui_message()
{
    // Read another incoming UDP packet.
    size_t packet_size = udp_handler.read_incoming_packet();
    if (packet_size == 0) {
        // Treat empty UDP packet as an invalid message.
        return InvalidMessage();
    }

    types::message_id_t message_id = udp_handler.read_next_packet_element<types::message_id_t>();

    switch (message_id)
    {
    case clientGuiCodes::placeBomb:
        if (packet_size == 1) return PlaceBomb();
        break;
    case clientGuiCodes::placeBlock:
        if (packet_size == 1) return PlaceBlock();
        break;
    case clientGuiCodes::move:
        if (packet_size == 1 + sizeof(Move)) {
            uint8_t d_val = udp_handler.read_next_packet_element<u_int8_t>();
            if (d_val < 4) {
                Direction direction = static_cast<Direction>(d_val);
                return Move(direction);
            }
        }
    }
    return InvalidMessage();
}

void ClientMessageManager::send_server_message(Join& message)
{
    tcp_handler.send_element<types::message_id_t>(serverClientCodes::join);
    message.serialize(tcp_handler);
}

void ClientMessageManager::send_server_message(PlaceBomb&)
{
    tcp_handler.send_element<types::message_id_t>(serverClientCodes::placeBomb);
}

void ClientMessageManager::send_server_message(PlaceBlock&)
{
    tcp_handler.send_element<types::message_id_t>(serverClientCodes::placeBlock);
}

void ClientMessageManager::send_server_message(Move& message)
{
    tcp_handler.send_element<types::message_id_t>(serverClientCodes::move);
    message.serialize(tcp_handler);
}

// Ignore.
void ClientMessageManager::send_server_message(InvalidMessage&) {};

// Over UDP.
void ClientMessageManager::send_gui_message(Lobby& message)
{
    udp_handler.append_to_outcoming_packet<types::message_id_t>(guiClientCodes::lobby);
    message.serialize(udp_handler);

    // Send the packet.
    udp_handler.flush_outcoming_packet();
}

void ClientMessageManager::send_gui_message(Game& message)
{
    udp_handler.append_to_outcoming_packet<types::message_id_t>(guiClientCodes::game);
    message.serialize(udp_handler);

    // Send the packet.
    udp_handler.flush_outcoming_packet();
}

ServerMessageManager::ServerMessageManager(TCPHandler &tcp_handler_) : tcp_handler(tcp_handler_) {}

ClientMessage ServerMessageManager::read_client_message()
{
    types::message_id_t message_id = tcp_handler.read_element<types::message_id_t>();

    switch (message_id)
    {
    case serverClientCodes::join:
        return Join(tcp_handler);
    
    case serverClientCodes::placeBomb:
        return PlaceBomb();

    case serverClientCodes::placeBlock:
        return PlaceBlock();

    case serverClientCodes::move:
        return Move(tcp_handler);

    default:
        throw std::runtime_error("Unknown message received from the client!");
    }
}