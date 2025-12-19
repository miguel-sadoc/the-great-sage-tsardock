/**
 * @file Hud.cpp
 * @brief Heads-up display implementation (stats + state banners).
 *
 * Portfolio notes:
 *  - HUD is a pure presentation layer: it does not own gameplay state.
 *  - Game calls setStats() with authoritative values; Hud formats + draws them.
 *  - Banners are centered using local bounds so they remain centered if text changes.
 */

#include "ui/Hud.hpp"
#include <string> // std::to_string

void Hud::centerOrigin(sf::Text& t) {
    // Center origin using local bounds (accounts for glyph offsets via left/top).
    const auto b = t.getLocalBounds();
    t.setOrigin(b.left + b.width / 2.f,
                b.top  + b.height / 2.f);
}

void Hud::init(const sf::Font& font, float windowWidth) {
    // -------------------------------------------------------------------------
    // Persistent stats (top-left corner)
    // -------------------------------------------------------------------------

    // Score
    m_score.setFont(font);
    m_score.setCharacterSize(20);
    m_score.setFillColor(sf::Color::White);
    m_score.setPosition(20.f, 20.f);
    m_score.setString("SCORE: 0");

    // Lives
    m_lives.setFont(font);
    m_lives.setCharacterSize(20);
    m_lives.setFillColor(sf::Color::White);
    m_lives.setPosition(20.f, 50.f);
    m_lives.setString("LIVES: 3");

    // Wave
    m_wave.setFont(font);
    m_wave.setCharacterSize(20);
    m_wave.setFillColor(sf::Color::White);
    m_wave.setPosition(20.f, 80.f);
    m_wave.setString("WAVE: 1");

    // -------------------------------------------------------------------------
    // State banners (center screen)
    // -------------------------------------------------------------------------

    // Game Over banner
    m_gameOver.setFont(font);
    m_gameOver.setCharacterSize(28);
    m_gameOver.setFillColor(sf::Color::Red);
    m_gameOver.setString("GAME OVER - PRESS SPACE");
    m_gameOver.setPosition(windowWidth / 2.f, 400.f);
    centerOrigin(m_gameOver);

    // Next Wave banner
    m_nextWave.setFont(font);
    m_nextWave.setCharacterSize(28);
    m_nextWave.setFillColor(sf::Color::Green);
    m_nextWave.setString("NEXT WAVE - PRESS SPACE");
    m_nextWave.setPosition(windowWidth / 2.f, 400.f);
    centerOrigin(m_nextWave);
}

void Hud::setStats(int score, int lives, int wave) {
    // Convert numeric state into UI strings.
    // Keeping formatting here avoids spreading string code into gameplay.
    m_score.setString("SCORE: " + std::to_string(score));
    m_lives.setString("LIVES: " + std::to_string(lives));
    m_wave.setString("WAVE: " + std::to_string(wave));
}

void Hud::draw(sf::RenderWindow& window, game::Mode mode) const {
    // Always draw persistent HUD stats.
    window.draw(m_score);
    window.draw(m_lives);
    window.draw(m_wave);

    // Draw only the relevant banner for the current mode.
    if (mode == game::Mode::GameOver) window.draw(m_gameOver);
    if (mode == game::Mode::NextWave) window.draw(m_nextWave);
}
