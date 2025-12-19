#pragma once

/**
 * @file Alien.hpp
 * @brief Single alien entity (position + sprite + collision bounds + delegated shooting animation).
 *
 * Design philosophy:
 *  - Keep Alien small and self-contained.
 *  - AlienFormation decides *who* shoots (cooldown + shooter candidates).
 *  - Alien owns *how* the shot happens: animation timing and the exact frame when the bullet is spawned.
 *
 * Why shooting lives here:
 *  - Prevents AlienFormation from accumulating per-alien animation state.
 *  - Reduces coupling: formation logic stays focused on grid movement and shooter selection.
 *  - Keeps the "shoot sequence" close to the visual representation (sprite sheet frames).
 */

#include "core/Config.hpp"
#include "core/Rect.hpp"
#include "entities/Bullet.hpp"
#include <SFML/Graphics.hpp>
#include <vector>

struct Alien {

    // Alive flag is used for rendering, collisions, movement, and to cancel animations safely.
    bool alive;

    // Center position in world/screen space (sprite origin is kept centered).
    float x, y;

    // Gameplay dimensions (kept explicit for readability; values come from cfg).
    float width, height;

    // SFML sprite representing this alien.
    sf::Sprite sprite;

    /**
     * @brief Shoot animation state owned by the Alien.
     *
     * Frame schedule (d1, d2, d3):
     *  - shootRect1 for [0, d1)
     *  - shootRect2 for [d1, d1+d2)   -> bullet spawns once here
     *  - shootRect3 for [d1+d2, d1+d2+d3)
     *  - then return to idleRect
     */
    struct ShootAnim {
        bool  active{false};
        bool  bulletSpawned{false};
        float timer{0.f};

        // Duration per frame segment (seconds). Tune to taste.
        float d1{0.1f};
        float d2{0.15f};
        float d3{0.1f};
    } shootAnim;

    /**
     * @brief Constructs a living alien at a given center position.
     *
     * Notes:
     *  - The sprite origin is centered to keep movement/layout predictable.
     *  - x/y are treated as the logical center, matching sprite origin.
     */
    Alien(float startX, float startY, const sf::Texture& texture, const sf::IntRect& textureRect)
        : alive(true),
          x(startX),
          y(startY),
          width(cfg::AlienWidth),
          height(cfg::AlienHeight) {

        sprite.setTexture(texture);
        sprite.setTextureRect(textureRect);

        recenterSprite();
        sprite.setPosition(cfg::PixelSnap(x), cfg::PixelSnap(y));
    }

    // --- Movement (dt-driven where appropriate) ---

    void moveLeft(float dt, float speed) {
        x -= speed * dt;
        sprite.setPosition(cfg::PixelSnap(x), cfg::PixelSnap(y));
    }

    void moveRight(float dt, float speed) {
        x += speed * dt;
        sprite.setPosition(cfg::PixelSnap(x), cfg::PixelSnap(y));
    }

    /**
     * @brief Discrete step-down used by the formation (classic Space Invaders behavior).
     * Note: This is not dt-based on purpose; formation step-down is an event.
     */
    void moveDown(float verticalStep) {
        y += verticalStep;
        sprite.setPosition(cfg::PixelSnap(x), cfg::PixelSnap(y));
    }

    // --- Collision ---

    /**
     * @brief Returns an axis-aligned bounding box for collision checks.
     * Uses global bounds because the sprite is positioned in world/screen space.
     */
    Rect getBounds() const {
        auto g = sprite.getGlobalBounds();
        Rect r;
        r.left = g.left; r.right  = g.left + g.width;
        r.top  = g.top;  r.bottom = g.top  + g.height;
        return r;
    }

    // --- Shooting (delegated behavior) ---

    /**
     * @brief Starts a new shooting sequence.
     * The formation calls this after it selects this alien as a shooter.
     */
    void startShoot() {
        shootAnim.active = true;
        shootAnim.timer = 0.f;
        shootAnim.bulletSpawned = false;
    }

    /**
     * @brief Updates shooting animation and spawns a bullet on the "fire frame".
     *
     * Contract:
     *  - Returns true  -> shoot sequence finished (alien back to idle)
     *  - Returns false -> still animating
     *
     * Safety:
     *  - If alien dies mid-animation, the sequence is canceled cleanly.
     */
    bool updateShoot(float dt,
                     const sf::IntRect& idleRect,
                     const sf::IntRect& shootRect1,
                     const sf::IntRect& shootRect2,
                     const sf::IntRect& shootRect3,
                     std::vector<Bullet>& outBullets,
                     float bulletSpeed,
                     const sf::Texture& projectileTex,
                     sf::IntRect& projectileRect)
    {
        // Not currently shooting -> nothing to do.
        if (!shootAnim.active) return true;

        // If alien died mid-animation, cancel and snap back to idle.
        if (!alive) {
            sprite.setTextureRect(idleRect);
            recenterSprite();
            sprite.setPosition(cfg::PixelSnap(x), cfg::PixelSnap(y));

            shootAnim.active = false;
            shootAnim.timer = 0.f;
            shootAnim.bulletSpawned = false;
            return true;
        }

        // Drive animation timeline.
        shootAnim.timer += dt;

        const float t1 = shootAnim.d1;
        const float t2 = t1 + shootAnim.d2;
        const float t3 = t2 + shootAnim.d3;

        // Frame 1
        if (shootAnim.timer < t1) {
            sprite.setTextureRect(shootRect1);
        }
        // Frame 2 (fire frame)
        else if (shootAnim.timer < t2) {
            sprite.setTextureRect(shootRect2);

            // Spawn exactly one bullet per shooting sequence.
            if (!shootAnim.bulletSpawned) {
                Bullet b;

                // x/y are the alien center (sprite is centered),
                // so y + height/2 places the bullet near the lower edge.
                b.createAlienBulletAt(
                    x,
                    y + height / 2.f,
                    bulletSpeed,
                    projectileTex,
                    projectileRect
                );

                outBullets.push_back(b);
                shootAnim.bulletSpawned = true;
            }
        }
        // Frame 3
        else if (shootAnim.timer < t3) {
            sprite.setTextureRect(shootRect3);
        }
        // End sequence -> back to idle
        else {
            sprite.setTextureRect(idleRect);
            recenterSprite();
            sprite.setPosition(cfg::PixelSnap(x), cfg::PixelSnap(y));

            shootAnim.active = false;
            shootAnim.timer = 0.f;
            shootAnim.bulletSpawned = false;
            return true;
        }

        // Keep sprite centered even if frame rect sizes differ.
        recenterSprite();
        sprite.setPosition(cfg::PixelSnap(x), cfg::PixelSnap(y));
        return false;
    }

private:
    /**
     * @brief Centers the sprite origin using local bounds.
     * Important when switching texture rects with different sizes.
     */
    void recenterSprite() {
        const auto b = sprite.getLocalBounds();
        sprite.setOrigin(b.left + (b.width  - 1.f) / 2.f,
                         b.top  + (b.height - 1.f) / 2.f);
    }
};
