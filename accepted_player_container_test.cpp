#include <iostream>
#include <thread>
#include <assert.h>
#include "accepted_player_container.h"

#define NUM_THREADS 3
#define NUM_ADDS_PER_THREAD 1000
#define NUM_PLAYERS 10

std::atomic<bool> start = false;

void add_n_players(AcceptedPlayerContainer& container, size_t n)
{
    // Just to make sure that the threads start at roughly the same time.
    while (!start) {}

    size_t counter;
    for (size_t i = 0; i < n; i++)
    {
        try
        {
            Player p;
            container.add_new_player(p);
            counter++;
        }
        catch (const RejectedPlayerException& e)
        {
            // It is okay.
        }
    }
    std::cout << "Added " << counter << " players." << std::endl;
}

int main()
{
    AcceptedPlayerContainer container(NUM_PLAYERS);
    std::vector<std::thread> threads;

    for (size_t i = 0; i < NUM_THREADS; i++)
    {
        std::thread t{[&]{ add_n_players(container, NUM_ADDS_PER_THREAD); }};
        threads.push_back(std::move(t));
    }

    // Start the concurrent operations.
    start = true;
    
    for (size_t i = 0; i < NUM_THREADS; i++)
    {
        threads.at(i).join();
    }
    
    // It should return immediately.
    GameStarted m = container.return_when_target_players_joined();
    assert(m.players.size() == NUM_PLAYERS);

    try
    {
        // Should throw a runtime error.
        AcceptedPlayer ignored = container.get_accepted_player(NUM_PLAYERS);
        std::cerr << "Should have thrown an exception!\n";
        exit(EXIT_FAILURE);
    }
    catch(const std::runtime_error& e)
    {
        // It is the desired behaviour.
    }

    return 0;
}