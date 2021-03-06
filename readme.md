READMY is still going to be updated...

This is an implementation of a multiplayer Bomberman game.

The project consists of the following modules:
- concurrency - It provides data structures that are used by multiple threads concurrently. Synchronization of access to these data structures is achieved by condition variables and mutexes.
- network - It provides high level methods used for network communication, as well as defines the message protocol. Under the hood Linux Socket API, data buffering, template metaprogramming, low level bytes manipulation methods were used. In order to provide a high level of interactivity, the Nagle's TCP congestion algorithm was turned off.
- game_logic - It provides classes responsible for the game logic.
- config - It provides configuration of the game logic and the message protocol.
- test - It provides thread-safety tests.
