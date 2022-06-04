#include "message_manager.h"

ClientMessageManager::ClientMessageManager(TCPHandler &tcp_handler_, UDPHandler &udp_handler_) :
        tcp_handler(tcp_handler_), udp_handler(udp_handler_) {}

ServerMessage ClientMessageManager::read_server_message() {
    auto message_id = tcp_handler.read_element<types::message_id_t>();

    switch (message_id) {
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

InputMessage ClientMessageManager::read_gui_message() {
    // Read another incoming UDP packet.
    size_t packet_size = udp_handler.read_incoming_packet();
    if (packet_size == 0) {
        // Treat empty UDP packet as an invalid message.
        return InvalidMessage();
    }

    auto message_id = udp_handler.read_next_packet_element<types::message_id_t>();

    switch (message_id) {
        case clientGuiCodes::placeBomb:
            if (packet_size == 1) return PlaceBomb();
            break;
        case clientGuiCodes::placeBlock:
            if (packet_size == 1) return PlaceBlock();
            break;
        case clientGuiCodes::move:
            if (packet_size == 1 + sizeof(Move)) {
                auto d_val = udp_handler.read_next_packet_element<u_int8_t>();
                if (d_val < 4) {
                    auto direction = static_cast<Direction>(d_val);
                    return Move(direction);
                }
            }
    }
    return InvalidMessage();
}

void ClientMessageManager::send_server_message(const Join &message) {
    tcp_handler.send_element<types::message_id_t>(serverClientCodes::join);
    message.serialize(tcp_handler);
}

void ClientMessageManager::send_server_message(const PlaceBomb &) {
    tcp_handler.send_element<types::message_id_t>(serverClientCodes::placeBomb);
}

void ClientMessageManager::send_server_message(const PlaceBlock &) {
    tcp_handler.send_element<types::message_id_t>(serverClientCodes::placeBlock);
}

void ClientMessageManager::send_server_message(const Move &message) {
    tcp_handler.send_element<types::message_id_t>(serverClientCodes::move);
    message.serialize(tcp_handler);
}

// Ignore.
void ClientMessageManager::send_server_message(const InvalidMessage &) {};

// Over UDP.
void ClientMessageManager::send_gui_message(LobbyMessage &&message) {
    udp_handler.append_to_outcoming_packet<types::message_id_t>(guiClientCodes::lobby);
    message.serialize(udp_handler);

    // Send the packet.
    udp_handler.flush_outcoming_packet();
}

void ClientMessageManager::send_gui_message(GameMessage &&message) {
    udp_handler.append_to_outcoming_packet<types::message_id_t>(guiClientCodes::game);
    message.serialize(udp_handler);

    // Send the packet.
    udp_handler.flush_outcoming_packet();
}

ServerMessageManager::ServerMessageManager(TCPHandler::ptr &tcp_handler_) : tcp_handler(tcp_handler_) {}

ClientMessage ServerMessageManager::read_client_message() {
    auto message_id = tcp_handler->read_element<types::message_id_t>();

    switch (message_id) {
        case serverClientCodes::join:
            return Join(*tcp_handler);

        case serverClientCodes::placeBomb:
            return PlaceBomb();

        case serverClientCodes::placeBlock:
            return PlaceBlock();

        case serverClientCodes::move:
            return Move(*tcp_handler);

        default:
            throw std::runtime_error("Unknown message received from the client!");
    }
}

void ServerMessageManager::send_client_message(const Hello &message) {
    tcp_handler->send_element<types::message_id_t>(clientServerCodes::hello);
    message.serialize(*tcp_handler);
}

void ServerMessageManager::send_client_message(const AcceptedPlayer &message) {
    tcp_handler->send_element<types::message_id_t>(clientServerCodes::acceptedPlayer);
    message.serialize(*tcp_handler);
}

void ServerMessageManager::send_client_message(const GameStarted &message) {
    tcp_handler->send_element<types::message_id_t>(clientServerCodes::gameStarted);
    message.serialize(*tcp_handler);
}

void ServerMessageManager::send_client_message(const Turn &message) {
    tcp_handler->send_element<types::message_id_t>(clientServerCodes::turn);
    message.serialize(*tcp_handler);
}

void ServerMessageManager::send_client_message(const GameEnded &message) {
    tcp_handler->send_element<types::message_id_t>(clientServerCodes::gameEnded);
    message.serialize(*tcp_handler);
}

std::string ServerMessageManager::get_client_name() const {
    return tcp_handler->get_peer_name();
}