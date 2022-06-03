#include <iostream>
#include <memory>
#include <set>
#include <thread>
#include <csignal>
#include <vector>
#include <shared_mutex>
#include "connection_acceptor.h"
#include "network_handler.h"
#include "message_manager.h"
#include "config.h"
#include "parser.h"
#include "accepted_player_container.h"
#include "move_container.h"
#include "turn_container.h"

typedef std::unique_lock<std::shared_mutex>  WriteLock;
typedef std::shared_lock<std::shared_mutex>  ReadLock;

options_server settings;

struct shared_state
{   
    std::shared_mutex mutex;
    bool game_started;
    size_t game_version;
    AcceptedPlayerContainer::ptr accepted_players;
    MoveContainer::ptr move_container;
    TurnContainer::ptr turn_container;
} shared;

void reset_shared()
{
    WriteLock lock_guard(shared.mutex);
    shared.game_started = false;
    shared.game_version++;
    shared.accepted_players = std::make_shared<AcceptedPlayerContainer>(settings.players_count);
    shared.move_container = std::make_shared<MoveContainer>(settings.players_count);
    shared.turn_container = std::make_shared<TurnContainer>();
}

bool is_game_started()
{
    // Here the lock is used only to introduce the happens-before relationship.
    ReadLock lock_guard(shared.mutex);
    return shared.game_started;
}

bool is_game_valid(size_t version)
{
    ReadLock lock_guard(shared.mutex);
    return shared.game_version == version;
}

void handle_tcp_stream_in(ServerMessageManager::ptr manager) 
{
    ClientMessage msg;
    
    // Pointers to shared data structures.
    AcceptedPlayerContainer::ptr accepted_players;
    MoveContainer::ptr move_container;

    // Determines whether client's participation in the game should be updated.
    size_t last_game_version = 0;
    size_t current_game_version;

    // True if and only if the client successfully joined the current version of the game.
    bool joined_the_game = false;

    // Valid only if the client joined the game.
    types::player_id_t player_id;

    try {
        while (true) {
            msg = manager->read_client_message();

            // Get most recent data structures.
            {
                ReadLock lock_guard(shared.mutex);
                accepted_players = shared.accepted_players;
                move_container = shared.move_container;
                current_game_version = shared.game_version;
            }

            if (last_game_version != current_game_version) {
                // A new game was started.
                joined_the_game = false;
            }


            if (std::holds_alternative<Join>(msg)) {
                if (!joined_the_game) 
                {
                    Player player;
                    player.name = std::get<Join>(msg).name;
                    player.address = "To be added.";

                    try
                    {
                        player_id = accepted_players->add_new_player(player);
                        joined_the_game = true;
                    }
                    catch(const RejectedPlayerException& e)
                    {
                        // The client was rejected from joining the game.
                    }
                }
                else {
                    // The client already joined the game!
                }
            }
            else {
                if (joined_the_game) {
                    // Proceed only if the client joined the game.
                    move_container->update_slot(player_id, msg);
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        // Communication with the client failed.
        std::cerr << e.what() << '\n';
        return;
    }
}

void handle_tcp_stream_out(ServerMessageManager::ptr manager)
{
    try
    {
        Hello hello_message(settings);
        manager->send_client_message(hello_message);

        while (true) {
            AcceptedPlayerContainer::ptr accepted_players;
            TurnContainer

            // Get most recent structures with players and moves.
            {
                ReadLock lock_guard(shared.mutex);
                accepted_players = shared.accepted_players;
                move_container = shared.move_container;
                game_version = shared.game_version;
            }
        }
    }
    catch (const std::exception& e)
    {
        // Communication with the client failed.
        std::cerr << e.what() << '\n';
        return;
    }
}

void accept_new_connections(types::port_t port)
{
    ConnectionAcceptor acceptor(port, TCP_BACKLOG_SIZE);
    
    while (true)
    {
        // Accept another connection.
        try
        {
            std::cout << "Open for new connection\n";
            
            // Wait for another connection request.
            int new_connection_fd = acceptor.accept_another_connection();

            // Create message manager for the newly connected client.
            TCPHandler::ptr handler = std::make_shared<TCPHandler>(new_connection_fd, TCP_BUFF_SIZE);
            ServerMessageManager::ptr manager = std::make_shared<ServerMessageManager>(handler);

            // Create two threads for data streaming in and out of the server.
            std::thread thread_in{[=]{ handle_tcp_stream_in(manager); }};
            std::thread thread_out{[=]{ handle_tcp_stream_out(manager); }};
            thread_in.detach();
            thread_out.detach();

            std::cout << "New connection established\n";
        }
        catch(const TCPAcceptError& e)
        {
            // Continue execution.
            std::cerr << e.what() << '\n';   
        }
    }
}

int main(int argc, char* argv[])
{
    settings = parse_server(argc, argv);
    reset_shared();

    // Create thread for accepting new connection.
    std::thread thread_acceptor{[=]{ accept_new_connections(settings.port); }};



    while (true)
    {
        sleep(100);
    }
    thread_acceptor.join();

    return 0;
}