#ifndef MESSAGE_MANAGER
#define MESSAGE_MANAGER

#include "network_handler.h"
#include "messages.h"

class ClientMessageManager
{
public:
    ClientMessageManager(TCPHandler&, UDPHandler&);

    /**
     * @brief Reads another message from the server.
     *
     * @return ServerMessage Message from the server.
     */
    ServerMessage read_server_message();
    InputMessage read_gui_message();

    void send_server_message(Join&);
    void send_server_message(PlaceBomb&);
    void send_server_message(PlaceBlock&);
    void send_server_message(Move&);
    void send_server_message(InvalidMessage&);

    void send_gui_message(Lobby&);
    void send_gui_message(Game&);

    /* Delete copy constructor and copy assignment. */
    ClientMessageManager(ClientMessageManager const&) = delete;
    void operator=(ClientMessageManager const&) = delete;

private:
    TCPHandler& tcp_handler;
    UDPHandler& udp_handler;
};

#endif // MESSAGES_H