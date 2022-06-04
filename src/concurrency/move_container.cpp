#include "move_container.h"

MoveContainer::MoveContainer(types::players_count_t num_slots)
{
    for (size_t i = 0; i < num_slots; i++)
    {
        ClientMessage m;
        slots.push_back({false, m});
    }
}

MoveContainer::container_t MoveContainer::atomic_snapshot_and_clear()
{
    std::unique_lock<std::mutex> lock_guard(mutex);

    container_t copy = slots;

    for (size_t i = 0; i < slots.size(); i++)
    {
        // Mark as not updated since last snapshot.
        slots.at(i).first = false;
    }

    return copy;
}

void MoveContainer::update_slot(types::player_id_t slot_id, const ClientMessage& move)
{
    std::unique_lock<std::mutex> lock_guard(mutex);

    if (slot_id >= slots.size()) {
        throw std::runtime_error("Trying to access invalid slot!");
    }

    // Mark as updated since last snapshot.
    slots.at(slot_id).first = true;
    slots.at(slot_id).second = move;
}