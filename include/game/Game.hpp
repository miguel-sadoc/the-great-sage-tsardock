#pragma once

/**
 * @file Game.hpp
 * @brief High-level gameplay orchestrator (input -> update -> render).
 *
 * Portfolio summary:
 *  - Game is the "composition root": it wires systems + entities together and applies rules.
 *  - Entities (Player, AlienFormation, Bullet) stay focused and reusable.
 *  - The simulation is dt-driven for stable behavior across different frame rates.
 *
 * Responsibilities:
 *  - Own the main loop and dispatch per-frame steps (processEvents, update, render).
 *  - Manage the global game state (Running / NextWave / GameOver).
 *  - Apply gameplay rules:
 *      - shooting (contextual SPACE)
 *      - collisions (bullets vs aliens, bullets vs player)
 *      - score / lives / wave progression
 *  - Own asset loading (via Resources) and HUD rendering (via Hud).
 */

#include <SFML/Graphics.hpp>
#include <vector>

#include "core/Config.hpp"
#include "entities/Bullet.hpp"
#include "entities/Player.hpp"
#include "game/AlienFormation.hpp"
#include "game/Mode.hpp"
#include "systems/Resources.hpp"
#include "ui/Hud.hpp"

class Game {
public:
    /**
     * @brief Initializes window, loads assets, and sets initial game state.
     *
     * Notes:
     *  - Heavy lifting of asset loading is delegated to Resources.
     *  - Entities are initialized here so they can be treated as "pure gameplay objects" later.
     */
    Game();

    /// @brief Runs the main loop until the window is closed.
    void run();

private:
    // -------------------------------------------------------------------------
    // Main loop steps
    // -------------------------------------------------------------------------

    /**
     * @brief Polls SFML events and handles input.
     *
     * Goal:
     *  - Keep this function mostly about input collection and dispatching commands.
     *  - Game rules should live in update() and smaller helper functions.
     */
    void processEvents(float dt);

    /**
     * @brief Advances simulation by dt and applies gameplay rules.
     *
     * Typical flow:
     *  - update movement (player/formation)
     *  - update bullets
     *  - resolve collisions
     *  - handle wave transitions / game over
     *  - update HUD state
     */
    void update(float dt);

    /// @brief Updates HUD texts/values from current score/lives/wave.
    void updateHud();

    /// @brief Renders background, entities, bullets and HUD according to current game mode.
    void render();

    // -------------------------------------------------------------------------
    // State transitions
    // -------------------------------------------------------------------------

    /// @brief Full restart: resets score, wave, lives, bullets, and formation.
    void restart();

    /// @brief Advances to the next wave and ramps difficulty (speed up / cooldown down).
    void goForNextWave();

    // -------------------------------------------------------------------------
    // Input actions (contextual SPACE)
    // -------------------------------------------------------------------------

    /**
     * @brief SPACE key behavior depends on the current mode:
     *  - NextWave  -> goForNextWave()
     *  - GameOver  -> restart()
     *  - Running   -> tryShoot()
     */
    void onSpacePressed();

    /// @brief Fires a player bullet if cooldown allows (Player owns its own cooldown).
    void tryShoot();

    // -------------------------------------------------------------------------
    // Update helpers (keep update() readable)
    // -------------------------------------------------------------------------

    /// @brief Moves bullets and removes those that leave the screen.
    void updateBullets(float dt);

    /// @brief Handles player bullets hitting aliens (score++, difficulty ramp, next wave trigger).
    void resolvePlayerBulletsVsAliens();

    /// @brief Handles alien bullets hitting player (life--, possibly game over).
    void resolveAlienBulletsVsPlayer();

    /**
     * @brief Updates projectile sprite animation for alien bullets (spritesheet cycling).
     * Note: This is purely visual; bullet physics is handled by Bullet::update().
     */
    void animateAlienBullets(float dt);

    // -------------------------------------------------------------------------
    // Render helpers
    // -------------------------------------------------------------------------

    /// @brief Draws all alien bullets.
    void drawAlienBullets();

    /// @brief Draws all player bullets.
    void drawPlayerBullets();

    // -------------------------------------------------------------------------
    // Asset / spritesheet setup
    // -------------------------------------------------------------------------

    /**
     * @brief Defines the projectile IntRects (frames) used by animateAlienBullets().
     * This keeps spritesheet slicing in one place.
     */
    void setAlienProjectileTexture(const sf::Texture& tex);

private:
    // -------------------------------------------------------------------------
    // Systems / infrastructure
    // -------------------------------------------------------------------------

    /// Centralized asset manager (textures, fonts, etc).
    Resources resources;

    /// HUD renderer and text layout.
    Hud hud;

    /// Main render target.
    sf::RenderWindow window;

    // -------------------------------------------------------------------------
    // Game state
    // -------------------------------------------------------------------------

    /// Single source of truth for the current game mode.
    game::Mode mode{game::Mode::Running};

    /// Score and wave progression.
    int score{0};
    int wave{1};

    // -------------------------------------------------------------------------
    // Player + bullets
    // -------------------------------------------------------------------------

    Player player;
    std::vector<Bullet> playerBullets;

    // -------------------------------------------------------------------------
    // Enemies + bullets
    // -------------------------------------------------------------------------

    /// Current difficulty parameters (scaled per wave).
    float alienCooldown{0.f};
    float alienSpeed{0.f};

    std::vector<Bullet> alienBullets;

    /// Formation controller: movement + shooter selection (shoot animation lives in Alien).
    AlienFormation aliens;

    // -------------------------------------------------------------------------
    // Sprites / visuals
    // -------------------------------------------------------------------------

    /// Background sprite (static).
    sf::Sprite bgSprite;

    /// Alien projectile animation frames (cycled in animateAlienBullets()).
    sf::Sprite alienProjectileSprite; // optional: remove if unused
    sf::IntRect alienProjectile1;
    sf::IntRect alienProjectile2;
    sf::IntRect alienProjectile3;
    sf::IntRect alienProjectile4;

    /// Player projectile sprite (optional: remove if unused if bullets draw themselves).
    sf::Sprite playerProjectileSprite; // optional: remove if unused
};
