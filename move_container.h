#ifndef MOVE_CONTAINER_H
#define MOVE_CONTAINER_H

#include <vector>
#include <utility>
#include <condition_variable>
#include <mutex>
#include <memory>
#include "config.h"
#include "messages.h"

class MoveContainer
{
public:
    /* Holds information about the last move and whether it was updated from the last snapshot. */
    using ptr = std::shared_ptr<MoveContainer>;
    using container_t = std::vector<std::pair<bool, ClientMessage>>;

    MoveContainer(types::players_count_t);

    container_t atomic_snapshot_and_clear();

    void update_slot(types::player_id_t, const ClientMessage&);

    /* Delete copy constructor and copy assignment. */
    MoveContainer(MoveContainer const &) = delete;
    void operator=(MoveContainer const &) = delete;

private:
    std::mutex mutex;
    container_t slots;
};

#endif // MOVE_CONTAINER_H