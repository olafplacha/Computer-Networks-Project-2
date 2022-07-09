/**
 * @author Olaf Placha
 * @brief This module provides a shared container used for storing moves requested by the players.
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef MOVE_CONTAINER_H
#define MOVE_CONTAINER_H

#include <vector>
#include <utility>
#include <condition_variable>
#include <mutex>
#include <memory>
#include "../config/config.h"
#include "../network/messages.h"

class MoveContainer {
public:
    /* Holds information about the last move and whether it was updated from the last snapshot. */
    using ptr = std::shared_ptr<MoveContainer>;
    using container_t = std::vector<std::pair<bool, ClientMessage>>;

    explicit MoveContainer(types::players_count_t);

    /**
     * @brief Atomically takes a snapshot of the container and resets it before the next round.
     * 
     * @return container_t - Snapshot of the container.
     */
    container_t atomic_snapshot_and_clear();

    /**
     * @brief Puts the move requested by the player in the container.
     * 
     */
    void update_slot(types::player_id_t, const ClientMessage &);

    /* Delete copy constructor and copy assignment. */
    MoveContainer(MoveContainer const &) = delete;

    void operator=(MoveContainer const &) = delete;

private:
    std::mutex mutex;
    container_t slots;
};

#endif // MOVE_CONTAINER_H