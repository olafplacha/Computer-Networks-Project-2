#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstring>
#include <thread>
#include <atomic>

#include "config.h"
#include "parser.h"
#include "game.h"
#include "message_manager.h"

enum State { LOBBY, GAME };
std::atomic<State> state;
std::atomic<bool> join_sent;

void guiClientServerStream(ClientMessageManager& manager, std::string& player_name)
{
    try
    {
        Join join(player_name);

        while (true) {
            InputMessage message = manager.read_gui_message();
            if (std::holds_alternative<InvalidMessage>(message)) {
                // Ignore.
                continue;
            }
            if (state == LOBBY && !join_sent) {
                // Send a request to join the game.
                manager.send_server_message(join);
                join_sent = true;
            }
            else {
                // Send next instruction input to the server.
                std::visit([&](auto&& arg) {
                    manager.send_server_message(arg);
                }, message);
            }
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        exit(EXIT_FAILURE);
    }
}

void serverClientGuiStream(ClientMessageManager& manager)
{
    try {
        ServerMessage message;
        message = manager.read_server_message();
        if (!std::holds_alternative<Hello>(message)) {
            throw std::runtime_error("Hello expected as the first message from the server!");
        }

        Hello hello = std::get<Hello>(message);

        while (true) {
            // New game is about to be started.
            LobbyMessage lobby(hello);
            manager.send_gui_message(lobby);

            while (state == LOBBY) {
                message = manager.read_server_message();
                if (std::holds_alternative<GameStarted>(message)) {
                    // Start the game!
                    state = GAME;
                }
                else if (std::holds_alternative<AcceptedPlayer>(message)) {
                    // Another player joined!
                    AcceptedPlayer player = std::get<AcceptedPlayer>(message);
                    lobby.accept(player);
                    // Send it tu gui.
                    manager.send_gui_message(lobby);
                }
                else {
                    std::runtime_error("Forbidden message received while in the lobby!");
                }
            }

            // Create a new game.
            GameStarted game_started = std::get<GameStarted>(message);
            GameClient game(hello, game_started);

            while (state == GAME) {
                message = manager.read_server_message();
                if (std::holds_alternative<GameEnded>(message)) {
                    // The game ended!
                    state = LOBBY;
                    join_sent = false;
                }
                else if (std::holds_alternative<Turn>(message)) {
                    Turn turn = std::get<Turn>(message);
                    game.apply_turn(turn);
                    // Get the same state and send it to gui.
                    GameMessage state = game.get_game_state();
                    manager.send_gui_message(state);
                }
            }
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[])
{
    // Parse program arguments.
    options_client op = parse_client(argc, argv);

    // Set up message manager.
    TCPHandler tcp_handler(op.server_address, op.server_port, TCP_BUFF_SIZE);
    UDPHandler udp_handler(op.port, op.gui_address, op.gui_port, UDP_BUFF_SIZE);
    ClientMessageManager manager(tcp_handler, udp_handler);

    // Set initial state.
    state = LOBBY;
    join_sent = false;

    // Create threads for (gui -> client -> server) and (server -> client -> gui) communication.
    std::thread t0{[&manager, &op]{ guiClientServerStream(manager, op.player_name); }};
    std::thread t1{[&manager]{ serverClientGuiStream(manager); }};

    t0.join();
    t1.join();

    return 0;
}