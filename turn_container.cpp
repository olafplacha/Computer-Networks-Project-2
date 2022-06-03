#include "turn_container.h"

void TurnContainer::append_new_turn(const Turn& turn)
{
    std::unique_lock<std::mutex> lock_guard(mutex);

    turns.push_back(turn);

    // Notify waiting threads about the new turn.
    condition_variable.notify_all();
}

Turn TurnContainer::get_turn(types::turn_t turn_id)
{
    std::unique_lock<std::mutex> lock_guard(mutex);

    // Wait until the turn is available.
    condition_variable.wait(lock_guard, [&]{ return turns.size() > turn_id; });

    // At this point the specified turn is available.
    return turns.at(turn_id);
}

void TurnContainer::return_when_game_finished()
{
    std::unique_lock<std::mutex> lock_guard(mutex);

    // Wait until the game is finished.
    condition_variable.wait(lock_guard, [&]{ return finished; });
}

void TurnContainer::mark_the_game_as_finished()
{
    std::unique_lock<std::mutex> lock_guard(mutex);

    finished = true;

    // Notify waiting threads about the end of the game.
    condition_variable.notify_all();
}