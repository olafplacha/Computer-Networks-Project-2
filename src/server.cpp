#include <iostream>
#include <memory>
#include <set>
#include <thread>
#include <csignal>
#include <vector>
#include <chrono>
#include <shared_mutex>
#include "network/connection_acceptor.h"
#include "network/network_handler.h"
#include "network/message_manager.h"
#include "config/config.h"
#include "config/parser.h"
#include "concurrency/accepted_player_container.h"
#include "concurrency/move_container.h"
#include "concurrency/turn_container.h"
#include "game_logic/game.h"

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

    // True if and only if the client successfully joined the most recent version of the game.
    bool joined_the_game = false;

    // Valid only if the client joined the most recent version of the game.
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
                last_game_version = current_game_version;
                joined_the_game = false;
            }

            if (std::holds_alternative<Join>(msg)) {
                if (!joined_the_game)
                {
                    Player player;
                    player.name = std::get<Join>(msg).name;
                    player.address = manager->get_client_name();

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
                if (joined_the_game && is_game_started()) {
                    // Proceed only if the client joined the most recent version of the game and the game is underway.
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
            TurnContainer::ptr turn_container;

            // Get most recent structures with players and moves.
            {
                ReadLock lock_guard(shared.mutex);
                accepted_players = shared.accepted_players;
                turn_container = shared.turn_container;
            }

            if (!is_game_started()) {
                // Show accepted players.
                for (types::player_id_t i = 0; i < settings.players_count; i++)
                {
                    AcceptedPlayer message = accepted_players->get_accepted_player(i);
                    manager->send_client_message(message);
                }
            }

            // Send message about the start of the game.
            GameStarted message = accepted_players->return_when_target_players_joined();
            manager->send_client_message(message);

            for (types::turn_t i = 0; i < settings.game_length + 1; i++)
            {
                // Wait for each turn to complete and send it.
                Turn message = turn_container->get_turn(i);
                manager->send_client_message(message);
            }

            // Send message about the end of the game.
            GameEnded message_end;
            Game::score_map_t score_map = turn_container->return_when_game_finished();
            message_end.scores = score_map;
            manager->send_client_message(message_end);
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
        AcceptedPlayerContainer::ptr accepted_players;
        MoveContainer::ptr move_container;
        TurnContainer::ptr turn_container;

        {
            ReadLock lock_guard(shared.mutex);
            accepted_players = shared.accepted_players;
            move_container = shared.move_container;
            turn_container = shared.turn_container;
        }

        // Wait until enough players join.
        accepted_players->return_when_target_players_joined();

        {
            WriteLock lock_guard(shared.mutex);
            shared.game_started = true;
        }

        // Initialize the game.
        GameServer game(settings);
        Turn turn = game.game_init();
        turn_container->append_new_turn(turn);

        // Carry out all the turns.
        for (types::turn_t i = 0; i < settings.game_length; i++)
        {
            turn = game.apply_moves(*move_container);
            turn_container->append_new_turn(turn);
        }

        // Get the score map after the game;
        Game::score_map_t score_map = game.get_score_map();

        // Prepare data structures for the next round.
        reset_shared();

        // Mark the last game as finished.
        turn_container->mark_the_game_as_finished(score_map);
    }

    thread_acceptor.join();

    return 0;
}