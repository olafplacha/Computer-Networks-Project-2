/**
 * @author Olaf Placha
 * @brief This module provides classes used for handling communication between server, clients and gui.
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef MESSAGE_MANAGER_H
#define MESSAGE_MANAGER_H

#include "network_handler.h"
#include "messages.h"

/**
 * @brief This class handles all communication between the client and the server as well as between
 * the client and the gui.
 */
class ClientMessageManager {
public:
    ClientMessageManager(TCPHandler &, UDPHandler &);

    /**
     * @brief Reads another message from the server.
     *
     * @return ServerMessage - Message from the server.
     */
    ServerMessage read_server_message();

    /**
     * @brief Reads another message from the gui.
     * 
     * @return InputMessage - Message from the gui.
     */
    InputMessage read_gui_message();

    /* Below there are overloaded methods used for sending various message types. */
    void send_server_message(const Join &);

    void send_server_message(const PlaceBomb &);

    void send_server_message(const PlaceBlock &);

    void send_server_message(const Move &);

    void send_server_message(const InvalidMessage &);

    void send_gui_message(LobbyMessage &&);

    void send_gui_message(GameMessage &&);

    /* Delete copy constructor and copy assignment. */
    ClientMessageManager(ClientMessageManager const &) = delete;

    void operator=(ClientMessageManager const &) = delete;

private:
    TCPHandler &tcp_handler;
    UDPHandler &udp_handler;
};

/**
 * @brief This class handles all communication between the server and the clients.
 */
class ServerMessageManager {
public:
    using ptr = std::shared_ptr<ServerMessageManager>;

    explicit ServerMessageManager(TCPHandler::ptr &);

    /**
     * @brief Reads another message from the client.
     * 
     * @return ClientMessage - Message from the client.
     */
    ClientMessage read_client_message();

    /* Below there are overloaded methods used for sending various message types. */
    void send_client_message(const Hello &);

    void send_client_message(const AcceptedPlayer &);

    void send_client_message(const GameStarted &);

    void send_client_message(const Turn &);

    void send_client_message(const GameEnded &);

    [[nodiscard]] std::string get_client_name() const;

    /* Delete copy constructor and copy assignment. */
    ServerMessageManager(ServerMessageManager const &) = delete;

    void operator=(ServerMessageManager const &) = delete;

private:
    TCPHandler::ptr tcp_handler;
};

#endif // MESSAGE_MANAGER_H