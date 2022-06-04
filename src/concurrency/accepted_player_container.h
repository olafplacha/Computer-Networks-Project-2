#ifndef ACCEPTED_PLAYER_CONTAINER_H
#define ACCEPTED_PLAYER_CONTAINER_H

#include <vector>
#include <stdexcept>
#include <condition_variable>
#include <mutex>
#include "../config/config.h"
#include "../network/messages.h"

class RejectedPlayerException : public std::logic_error {
public:
    explicit RejectedPlayerException(const char *w) : std::logic_error(w) {}
};

class AcceptedPlayerContainer {
public:
    using ptr = std::shared_ptr<AcceptedPlayerContainer>;

    explicit AcceptedPlayerContainer(types::players_count_t);

    /* Return when enough players join, so that the game can be started. It returns no sooner than the game starts. */
    GameStarted return_when_target_players_joined();

    /**
     * @brief Adds a new player to the container.
     *
     * @throws RejectedPlayerException Throws if there is already a full set of players.
     */
    types::player_id_t add_new_player(const Player &);

    /* Return message about specific accepted player when it is ready. */
    AcceptedPlayer get_accepted_player(types::player_id_t);

    /* Delete copy constructor and copy assignment. */
    AcceptedPlayerContainer(AcceptedPlayerContainer const &) = delete;

    void operator=(AcceptedPlayerContainer const &) = delete;

private:
    std::mutex mutex;
    std::condition_variable condition_variable;
    std::map<types::player_id_t, Player> accepted_players;
    types::players_count_t target_players_count;
};

#endif // ACCEPTED_PLAYER_CONTAINER_H