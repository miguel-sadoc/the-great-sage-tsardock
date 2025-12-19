#pragma once

/**
 * @file AlienFormation.hpp
 * @brief Manages the alien grid as a single cohesive formation (layout, movement, shooter selection).
 *
 * Responsibilities:
 *  - Own the alien grid (rows x cols) and update it as a classic Space Invaders formation:
 *      horizontal drift -> edge detection -> synchronized step-down -> direction flip.
 *  - Maintain shooter candidates: for each column, only the lowest living alien can shoot.
 *  - Provide high-level operations (move, reset, collision checks) without exposing grid internals to Game.
 *
 * Separation of concerns:
 *  - Game:
 *      - owns score/waves/game state and global progression rules
 *      - calls AlienFormation for movement/collisions/shooting updates
 *  - AlienFormation:
 *      - owns formation movement + shooter selection pacing (cooldown + candidate list)
 *  - Alien:
 *      - owns the shoot animation timing and bullet spawn moment (delegated behavior)
 */

#include <SFML/Graphics.hpp>
#include <random>
#include <vector>

#include "core/Rect.hpp"
#include "entities/Alien.hpp"
#include "entities/Bullet.hpp"

class AlienFormation {
public:
    /**
     * @param minX, maxX Horizontal world/screen limits used as formation boundaries.
     * @param xSpeed Initial horizontal speed in pixels/second.
     * @param initialCooldown Time between shots (seconds), used for pacing.
     */
    AlienFormation(float minX, float maxX, float xSpeed, float initialCooldown);

    /**
     * Updates formation movement.
     *
     * Algorithm:
     *  1) Scan living aliens to detect if the next horizontal step would exceed boundaries.
     *  2) If so, schedule a synchronized step-down for all living aliens and flip direction.
     *  3) Otherwise, move all living aliens horizontally using dt-based motion.
     *
     * @param dt Delta time (seconds).
     * @param verticalLimit Lowest allowed Y (e.g., player's top). Crossing it => game over.
     * @return false if a step-down would cross verticalLimit (used as a game-over trigger).
     */
    bool move(float dt, float verticalLimit);

    /**
     * Bullet vs formation collision test.
     * If a living alien is hit, it is marked dead and shooter candidates are rebuilt.
     *
     * @param bulletBounds AABB of the bullet (world/screen space).
     * @return true if a hit occurred.
     */
    bool hasBeenHit(Rect bulletBounds);

    /**
     * Safety check: returns true if any living alien crosses the bottom of the screen.
     * (Usually the verticalLimit in move() is the primary loss condition.)
     */
    bool hasReachedBottom(float windowHeight) const;

    /// Returns true if at least one alien is still alive.
    bool hasAliveAliens() const;

    /// Decrements alive counter (Game calls this after a successful hit).
    void decreaseAliveAliens();

    /// Small intra-wave speed ramp (classic arcade pressure increase).
    void increaseHorizontalSpeed();

    /**
     * Cooldown-based shooter selection.
     *
     * Important:
     *  - This returns an alien index eligible to shoot, or -1 if not ready.
     *  - Bullet spawn is NOT handled here; it is delegated to Alien::updateShoot()
     *    during the appropriate animation frame.
     */
    int shoot(float dt);

    /**
     * Resets formation state for a new wave (Game provides updated speed/cooldown).
     * Rebuilds aliens and shooter candidates.
     */
    void reset(float initialXSpeed, float initialCooldown);

    /// Draws all living aliens.
    void draw(sf::RenderWindow& window) const;

    /**
     * Sets the alien spritesheet and frame rectangles (idle + shoot animation frames).
     * Must be called before initializeAliens/reset so created aliens have valid sprites.
     */
    void setTexture(const sf::Texture& tex);

    /**
     * Updates formation shooting (delegated approach).
     *
     * Behavior:
     *  - If no alien is currently shooting, pick a shooter index (cooldown + shooterList).
     *  - Start its shoot animation (Alien::startShoot()).
     *  - Advance its animation (Alien::updateShoot()) and spawn exactly one bullet on the fire frame.
     *
     * @param alienBullets Output container for spawned alien bullets.
     * @param bulletSpeed Projectile speed (positive typically means downward).
     * @param dt Delta time (seconds).
     * @param projectileTex Projectile spritesheet texture.
     * @param projectileRect Projectile frame rect (passed through to Bullet API).
     */
    void update(std::vector<Bullet>& alienBullets,
                float bulletSpeed,
                float dt,
                const sf::Texture& projectileTex,
                sf::IntRect& projectileRect);

private:
    /// Factory helper to centralize alien creation (keeps initialization clean).
    Alien createAlienAt(float X, float Y);

    /// Builds the full grid and initializes the initial shooter candidates.
    void initializeAliens();

    /**
     * Rebuilds shooterList: for each column, keep only the lowest living alien.
     * This matches classic Space Invaders behavior and avoids "shooting through" aliens.
     */
    void updateShooterList();

    /// Randomly choose one index from shooterList using a modern RNG.
    int chooseRandomAlien() const;

private:
    // --- Shooting pacing (selection only; animation lives in Alien) ---
    float timeSinceLastShot{0.f};
    float cooldown{3.f};

    // --- Formation movement ---
    float horizontalSpeed{0.f};

    /// Discrete step-down distance (pixels) applied after an edge hit (arcade "event" movement).
    float verticalStep{10.f};

    float maxHorizontalX{0.f};
    float minHorizontalX{0.f};

    // --- Grid ---
    int rows{5};
    int cols{8};
    int aliveAliens{0};

    // --- Movement state ---
    bool moveDownFlag{false};
    bool movingRight{true};

    // --- Data ---
    std::vector<Alien> aliens;

    /// For each column, stores the index of the lowest living alien (eligible shooter).
    std::vector<int> shooterList;

    // --- RNG (modern replacement for std::rand) ---
    mutable std::mt19937 rng;

    // --- Sprite sheet data ---
    const sf::Texture* alienTexture{nullptr};
    sf::IntRect idleRect{};
    sf::IntRect shootRect1{};
    sf::IntRect shootRect2{};
    sf::IntRect shootRect3{};

    /// Tracks the single alien currently executing the shoot animation (-1 means none).
    int shootingAlienIndex{-1};
};
