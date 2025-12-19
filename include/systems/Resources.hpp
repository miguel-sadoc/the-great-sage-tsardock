#pragma once

/**
 * @file Resources.hpp
 * @brief Minimal asset manager (fonts/textures) for the game.
 *
 * Portfolio notes:
 *  - Centralizes asset loading to keep gameplay classes (Game/Player/AlienFormation) clean.
 *  - Provides read-only accessors to already-loaded assets (no ownership leaks).
 *  - loadAll() is a single entry point so Game can fail fast if something is missing.
 *
 * Design choices:
 *  - Assets are stored by value (sf::Texture/sf::Font) and exposed by const reference.
 *    This keeps lifetime management simple: Resources owns everything for the entire run.
 *  - Private helpers (loadFont/loadTexture) keep file IO/error handling consistent.
 */

#include <SFML/Graphics.hpp>
#include <string>

class Resources {
public:
    /**
     * @brief Loads all assets required by the game.
     *
     * Typical usage:
     *  - Game calls resources.loadAll() once during initialization.
     *  - If loadAll() fails, the game can log an error and still run with fallbacks
     *    (or exit gracefully, depending on your preference).
     *
     * @return true if all assets were loaded successfully.
     */
    bool loadAll();

    // -------------------------------------------------------------------------
    // Read-only accessors
    // -------------------------------------------------------------------------
    // Expose assets as const references to prevent accidental copies and mutation.

    const sf::Font& font() const {
        return m_font;
    }

    const sf::Texture& playerProjectile() const {
        return m_playerProjectile;
    }

    const sf::Texture& playerSheet() const {
        return m_playerSheet;
    }

    const sf::Texture& background() const {
        return m_background;
    }

    const sf::Texture& alienSheet() const {
        return m_alienSheet;
    }

    const sf::Texture& alienProjectileSheet() const {
        return m_alienProjectileSheet;
    }

private:
    // -------------------------------------------------------------------------
    // Internal helpers
    // -------------------------------------------------------------------------

    /**
     * @brief Loads a font from disk.
     * Centralized here so all loads can share consistent error handling/logging.
     */
    bool loadFont(sf::Font& f, const std::string& path);

    /**
     * @brief Loads a texture from disk.
     * Centralized here so all loads can share consistent error handling/logging.
     */
    bool loadTexture(sf::Texture& t, const std::string& path);

private:
    // -------------------------------------------------------------------------
    // Stored assets (owned for the duration of the program)
    // -------------------------------------------------------------------------

    sf::Font    m_font;

    sf::Texture m_playerProjectile;
    sf::Texture m_playerSheet;
    sf::Texture m_background;
    sf::Texture m_alienSheet;
    sf::Texture m_alienProjectileSheet;
};
