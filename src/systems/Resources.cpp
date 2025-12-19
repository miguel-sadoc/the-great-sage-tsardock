/**
 * @file Resources.cpp
 * @brief Asset loading implementation (fonts + textures).
 *
 * Portfolio notes:
 *  - Resources centralizes all file I/O related to assets.
 *  - Game code stays clean: it just calls loadAll() once and then requests references.
 *  - Each loader function logs a clear error message so missing assets are easy to diagnose.
 *
 * Implementation details:
 *  - loadAll() returns a single boolean that represents whether *all* assets loaded successfully.
 *  - We use `ok &= ...` to accumulate failures while still attempting to load everything:
 *      - This prints all missing/invalid paths in one run (better debugging experience).
 */

#include "systems/Resources.hpp"
#include <iostream>

bool Resources::loadFont(sf::Font& f, const std::string& path) {
    // Small wrapper to keep error logging consistent across the codebase.
    if (!f.loadFromFile(path)) {
        std::cerr << "[Resources] Failed to load font: " << path << "\n";
        return false;
    }
    return true;
}

bool Resources::loadTexture(sf::Texture& t, const std::string& path) {
    // Small wrapper to keep error logging consistent across the codebase.
    if (!t.loadFromFile(path)) {
        std::cerr << "[Resources] Failed to load texture: " << path << "\n";
        return false;
    }

    // Optional: for pixel art, you often want smoothing disabled.
    // If you prefer crisp pixels, keep this:
    // t.setSmooth(false);

    return true;
}

bool Resources::loadAll() {
    /**
     * Load all assets required by the game.
     *
     * Why we keep paths here:
     *  - One place to update asset filenames/locations.
     *  - Easy to review what the game depends on.
     *
     * Note:
     *  - `ok &= ...` continues loading after failures so you see *all* missing files at once.
     */
    bool ok = true;

    ok &= loadFont(m_font, "assets/fonts/Orbitron-VariableFont_wght.ttf");

    ok &= loadTexture(m_playerProjectile,      "assets/sprites/playerProjectile.png");
    ok &= loadTexture(m_playerSheet,           "assets/sprites/player_sheet.png");
    ok &= loadTexture(m_background,            "assets/sprites/background.png");
    ok &= loadTexture(m_alienSheet,            "assets/sprites/alien_sheet.png");
    ok &= loadTexture(m_alienProjectileSheet,  "assets/sprites/AlienProjectileSheet.png");

    if (!ok) {
        std::cerr << "[Resources] One or more assets failed to load. "
                     "The game may run with missing visuals.\n";
    }

    return ok;
}
