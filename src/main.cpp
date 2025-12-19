/**
 * @file main.cpp
 * @brief Minimal application entry point.
 *
 * Portfolio notes:
 *  - main() is intentionally tiny: it just boots the game and hands off control.
 *  - The full loop (input -> update -> render) lives inside Game.
 *
 * Randomness:
 *  - This project uses modern RNG (std::mt19937) inside AlienFormation,
 *    so there is no need to seed std::rand()/std::srand() here.
 */

#include "game/Game.hpp"

int main() {
    Game game;
    game.run();
    return 0;
}
