#pragma once

/**
 * @file Hud.hpp
 * @brief Lightweight HUD renderer (score, lives, wave + state banners).
 *
 * Portfolio notes:
 *  - The HUD is kept separate from gameplay simulation.
 *  - Game owns the "truth" (score/lives/wave/mode); Hud only formats and draws.
 *  - This separation makes it easy to change UI without touching game rules.
 *
 * Responsibilities:
 *  - Initialize text elements (font, sizes, colors, positions).
 *  - Update displayed values (score/lives/wave).
 *  - Render overlay according to the current game mode (Running / NextWave / GameOver).
 */

#include <SFML/Graphics.hpp>
#include "game/Mode.hpp"

class Hud {
public:
    /**
     * @brief Initializes HUD texts (font, sizes, colors, anchor positions).
     * @param font Loaded SFML font (owned by Resources).
     * @param windowWidth Used to center the state banners (Game Over / Next Wave).
     */
    void init(const sf::Font& font, float windowWidth);

    /**
     * @brief Updates the numeric HUD values.
     * Called once per frame (or whenever values change).
     */
    void setStats(int score, int lives, int wave);

    /**
     * @brief Draws HUD elements on top of the scene.
     * The mode controls whether banners are shown.
     */
    void draw(sf::RenderWindow& window, game::Mode mode) const;

private:
    /**
     * @brief Centers the origin of a sf::Text using its local bounds.
     * This allows consistent centering when the string content changes.
     */
    static void centerOrigin(sf::Text& t);

private:
    // Persistent HUD stats (top-left).
    sf::Text m_score;
    sf::Text m_lives;
    sf::Text m_wave;

    // State banners (center screen).
    sf::Text m_gameOver;
    sf::Text m_nextWave;
};
