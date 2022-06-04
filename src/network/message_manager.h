#ifndef MESSAGE_MANAGER_H
#define MESSAGE_MANAGER_H

#include "network_handler.h"
#include "messages.h"

class ClientMessageManager {
public:
    ClientMessageManager(TCPHandler &, UDPHandler &);

    /**
     * @brief Reads another message from the server.
     *
     * @return ServerMessage Message from the server.
     */
    ServerMessage read_server_message();

    InputMessage read_gui_message();

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

class ServerMessageManager {
public:
    using ptr = std::shared_ptr<ServerMessageManager>;

    explicit ServerMessageManager(TCPHandler::ptr &);

    ClientMessage read_client_message();

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