#include "accepted_player_container.h"

AcceptedPlayerContainer::AcceptedPlayerContainer(types::players_count_t target_players_count_) :
        target_players_count(target_players_count_) {}

GameStarted AcceptedPlayerContainer::return_when_target_players_joined() {
    std::unique_lock<std::mutex> lock_guard(mutex);

    // Wait until full set of players join.
    condition_variable.wait(lock_guard, [&] { return target_players_count == accepted_players.size(); });

    GameStarted message;
    message.players = accepted_players;
    return message;
}

types::player_id_t AcceptedPlayerContainer::add_new_player(const Player &player) {
    std::unique_lock<std::mutex> lock_guard(mutex);

    if (target_players_count == accepted_players.size()) {
        throw RejectedPlayerException("Full set of players already exists!");
    }

    // Add a new player to the map.
    accepted_players.insert({accepted_players.size(), player});

    // Notify waiting threads about the new player.
    condition_variable.notify_all();

    return (types::player_id_t) (accepted_players.size() - 1);
}

AcceptedPlayer AcceptedPlayerContainer::get_accepted_player(types::player_id_t id) {
    std::unique_lock<std::mutex> lock_guard(mutex);

    if (id >= target_players_count) {
        throw std::runtime_error("Trying to get player thet will never exist!");
    }

    // Wait until the specified player is added.
    condition_variable.wait(lock_guard, [&] { return accepted_players.size() > id; });

    // At this point the player must exist.
    AcceptedPlayer player;
    player.id = id;
    player.player = accepted_players.at(id);

    return player;
}