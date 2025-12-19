/**
 * @file AlienFormation.cpp
 * @brief Space-Invaders-style formation controller (movement + shooter candidates + delegated shooting).
 *
 * High-level behavior:
 *  - Classic "block" movement:
 *      horizontal drift -> edge hit -> synchronized step-down -> direction flip.
 *  - Shooter candidates:
 *      only the lowest living alien in each column is eligible to shoot.
 *  - Delegated shooting:
 *      formation chooses *who* shoots (cooldown + random eligible shooter),
 *      but the selected Alien owns the *shoot animation* and the *exact bullet spawn frame*.
 *
 * Key detail (movement correctness):
 *  - moveDownFlag is treated like an "event".
 *    Once an edge collision is detected, we do NOT mix horizontal movement and step-down
 *    in the same frame for some aliens and not others. Instead, we schedule a single
 *    synchronized step-down for the entire formation on the next action.
 */

#include "game/AlienFormation.hpp"
#include "core/Config.hpp"

#include <cmath>
#include <random>

AlienFormation::AlienFormation(float minX, float maxX, float xSpeed, float initialCooldown)
    : timeSinceLastShot(0.f),
      cooldown(initialCooldown),
      horizontalSpeed(xSpeed),
      verticalStep(cfg::AliensVerticalStep),
      maxHorizontalX(maxX),
      minHorizontalX(minX),
      rows(cfg::AliensRows),
      cols(cfg::AliensCols),
      aliveAliens(rows * cols),
      moveDownFlag(false),
      movingRight(true),
      rng(std::random_device{}())
{
}

Alien AlienFormation::createAlienAt(float X, float Y) {
    // Requirement: setTexture() must be called before initializeAliens()/reset()
    // so alienTexture and animation rects are valid.
    return Alien(X, Y, *alienTexture, idleRect);
}

void AlienFormation::initializeAliens() {
    aliens.clear();
    shooterList.clear();

    // We create a "template" alien just to read consistent logical dimensions.
    // (width/height are stable and come from cfg::AlienWidth/AlienHeight in your Alien constructor.)
    Alien templateAlien = createAlienAt(0.f, 0.f);

    const float startX = cfg::AliensStartX;
    const float startY = cfg::AliensStartY;
    const float hGap   = cfg::AliensHorizontalGap;
    const float vGap   = cfg::AliensVerticalGap;

    aliens.reserve(rows * cols);

    // IMPORTANT:
    // Alien stores x/y as CENTER coordinates (sprite origin is centered),
    // so we build the grid in centered coordinates as well.
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            aliens.push_back(createAlienAt(
                startX + c * (templateAlien.width  + hGap) + templateAlien.width  / 2.f,
                startY + r * (templateAlien.height + vGap) + templateAlien.height / 2.f
            ));
        }
    }

    // Initial shooter candidates: bottom row, one per column.
    // After kills, updateShooterList() keeps only the lowest living alien per column.
    for (int c = 0; c < cols; c++) {
        shooterList.push_back((rows - 1) * cols + c);
    }

    // No active shooter at reset.
    shootingAlienIndex = -1;
}

void AlienFormation::updateShooterList() {
    // shooterList holds, for each column, the lowest living alien.
    // This matches classic Space Invaders and prevents "shooting through" aliens.
    shooterList.clear();

    for (int c = 0; c < cols; c++) {
        // Scan from bottom row to top row.
        for (int r = rows; r > 0; ) {
            const int idx = cols * (r - 1) + c;
            if (aliens[idx].alive) {
                shooterList.push_back(idx);
                break;
            }
            r--;
        }
    }
}

bool AlienFormation::move(float dt, float verticalLimit) {
    // Step 1) Scan living aliens to see if the NEXT horizontal movement would cross boundaries.
    // If so:
    //  - Flip direction
    //  - Schedule a synchronized step-down via moveDownFlag
    //  - Validate that step-down would not cross verticalLimit (game over trigger)

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            const int idx = cols * r + c;
            if (!aliens[idx].alive) continue;

            Rect b = aliens[idx].getBounds();

            if (movingRight && b.right + horizontalSpeed * dt > maxHorizontalX) {
                // If the formation steps down beyond player's vertical space, the game ends.
                if (b.bottom + verticalStep > verticalLimit) return false;

                movingRight  = false;
                moveDownFlag = true;
                break;
            }

            if (!movingRight && b.left - horizontalSpeed * dt < minHorizontalX) {
                if (b.bottom + verticalStep > verticalLimit) return false;

                movingRight  = true;
                moveDownFlag = true;
                break;
            }
        }

        if (moveDownFlag) break;
    }

    // Step 2) If an edge was hit, apply the discrete step-down to ALL living aliens.
    // This keeps the formation cohesive and avoids partial updates.
    if (moveDownFlag) {
        for (auto& a : aliens) {
            if (a.alive) a.moveDown(verticalStep);
        }
        moveDownFlag = false;
        return true;
    }

    // Step 3) Otherwise, drift horizontally (dt-based) for all living aliens.
    for (auto& a : aliens) {
        if (!a.alive) continue;

        if (movingRight) a.moveRight(dt, horizontalSpeed);
        else             a.moveLeft(dt, horizontalSpeed);
    }

    return true;
}

bool AlienFormation::hasBeenHit(Rect bulletBounds) {
    // Bullet vs formation collision:
    // - Mark hit alien dead
    // - Update shooterList so the next lowest alien in that column becomes eligible
    // - Return true so Game can apply score/progression and remove the bullet

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            const int idx = cols * r + c;
            Alien& a = aliens[idx];
            if (!a.alive) continue;

            if (bulletBounds.intersects(a.getBounds())) {
                a.alive = false;

                // If the current shooter died mid-animation, stop tracking it.
                // Alien::updateShoot() also cancels cleanly if alive=false.
                if (shootingAlienIndex == idx) shootingAlienIndex = -1;

                updateShooterList();
                return true;
            }
        }
    }

    return false;
}

bool AlienFormation::hasReachedBottom(float windowHeight) const {
    // Optional safety check: if any living alien passes the bottom, treat as loss.
    for (const auto& a : aliens) {
        if (!a.alive) continue;
        if (a.getBounds().bottom > windowHeight) return true;
    }
    return false;
}

bool AlienFormation::hasAliveAliens() const {
    return aliveAliens > 0;
}

void AlienFormation::decreaseAliveAliens() {
    // Game calls this after a confirmed hit (keeps scoring/progression in Game).
    aliveAliens--;
}

void AlienFormation::increaseHorizontalSpeed() {
    // Gentle intra-wave ramp to increase pressure.
    horizontalSpeed *= 1.05f;
}

int AlienFormation::chooseRandomAlien() const {
    // Modern RNG (uniform distribution) instead of std::rand().
    if (shooterList.empty()) return -1;

    std::uniform_int_distribution<int> dist(0, static_cast<int>(shooterList.size()) - 1);
    return shooterList[dist(rng)];
}

int AlienFormation::shoot(float dt) {
    // Cooldown-based shooter selection:
    // - Accumulate dt until cooldown is reached.
    // - When ready, choose a random eligible shooter from shooterList.
    // - Bullet spawning is NOT done here (delegated to Alien::updateShoot()).

    if (timeSinceLastShot < cooldown) {
        timeSinceLastShot += dt;
        return -1;
    }

    const int idx = chooseRandomAlien();
    if (idx == -1) return -1;

    timeSinceLastShot = 0.f;
    return idx;
}

void AlienFormation::reset(float initialXSpeed, float initialCooldown) {
    // Reset wave parameters (Game controls the difficulty scaling between waves).
    horizontalSpeed   = initialXSpeed;
    cooldown          = initialCooldown;
    timeSinceLastShot = 0.f;

    movingRight  = true;
    moveDownFlag = false;

    aliveAliens = rows * cols;

    initializeAliens();
}

void AlienFormation::setTexture(const sf::Texture& tex) {
    // Store a pointer to avoid copying textures.
    alienTexture = &tex;

    // Spritesheet rectangles: idle + 3-frame shooting animation.
    idleRect    = sf::IntRect(0,   0, 41, 35);
    shootRect1  = sf::IntRect(41,  2, 41, 32);
    shootRect2  = sf::IntRect(82,  5, 46, 27);
    shootRect3  = sf::IntRect(127, 2, 41, 32);

    // Recreate aliens so their sprites reference valid texture/rects immediately.
    initializeAliens();
}

void AlienFormation::update(std::vector<Bullet>& alienBullets,
                            float bulletSpeed,
                            float dt,
                            const sf::Texture& projectileTex,
                            sf::IntRect& projectileRect)
{
    // Delegated shooting:
    // - Formation decides WHICH alien shoots (cooldown + candidate list).
    // - Alien plays its shoot animation and spawns the bullet on the "fire frame".
    // - Only one alien shoots at a time (classic + simple).

    // 1) If no alien is currently shooting, attempt to start a new shoot sequence.
    if (shootingAlienIndex == -1) {
        const int idx = shoot(dt);

        if (idx < 0 || idx >= static_cast<int>(aliens.size())) return;
        if (!aliens[idx].alive) return;

        shootingAlienIndex = idx;
        aliens[shootingAlienIndex].startShoot();
    }

    // 2) Validate current shooter index.
    if (shootingAlienIndex < 0 || shootingAlienIndex >= static_cast<int>(aliens.size())) {
        shootingAlienIndex = -1;
        return;
    }

    // If shooter died, cancel and allow a new shooter later.
    if (!aliens[shootingAlienIndex].alive) {
        shootingAlienIndex = -1;
        return;
    }

    // 3) Advance the shooter animation; Alien spawns a bullet once during the fire frame.
    const bool finished = aliens[shootingAlienIndex].updateShoot(
        dt,
        idleRect,
        shootRect1,
        shootRect2,
        shootRect3,
        alienBullets,
        bulletSpeed,
        projectileTex,
        projectileRect
    );

    // 4) When the animation finishes, free the slot for the next shooter.
    if (finished) shootingAlienIndex = -1;
}

void AlienFormation::draw(sf::RenderWindow& window) const {
    // Draw only living aliens (dead ones remain in the array for stable indexing).
    for (const auto& a : aliens) {
        if (!a.alive) continue;
        window.draw(a.sprite);
    }
}
