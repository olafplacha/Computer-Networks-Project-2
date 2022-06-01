#include <vector>
#include <stdexcept>
#include <condition_variable>
#include <mutex>
#include "config.h"
#include "messages.h"

class RejectedPlayerException : public std::logic_error
{
public:
    RejectedPlayerException(const char *w) : std::logic_error(w) {}
};

class AcceptedPlayerContainer
{
public:
    AcceptedPlayerContainer(types::players_count_t);

    /* Return when enough players join, so that the game can be started. */
    GameStarted return_when_target_players_joined();

    /**
     * @brief Adds a new player to the container.
     *
     * @throws RejectedPlayerException Throws if there is already a full set of players.
     */
    void add_new_player(const Player &);

    /* Return message about specific accpeted player when it is ready. */
    AcceptedPlayer get_accepted_player(types::player_id_t);

    /* Delete copy constructor and copy assignment. */
    AcceptedPlayerContainer(AcceptedPlayerContainer const &) = delete;
    void operator=(AcceptedPlayerContainer const &) = delete;

private:
    std::mutex mutex;
    std::condition_variable condition_variable;
    std::map<types::player_id_t, Player> accepted_players;
    types::players_count_t target_players_count;
    types::players_count_t current_players_count;
};