/**
 * @file Game.cpp
 * @brief Main loop + gameplay rules (composition root).
 *
 * Portfolio notes:
 *  - Game owns the main loop and applies rules; entities remain focused and reusable.
 *  - The simulation is dt-driven (delta time) for consistent behavior across frame rates.
 *
 * Core rules:
 *  - SPACE is contextual:
 *      - NextWave  -> advance wave
 *      - GameOver  -> restart game
 *      - Running   -> attempt to shoot (Player cooldown)
 *  - Player bullet vs alien:
 *      - remove bullet, mark alien dead, score++, ramp difficulty slightly
 *  - Alien bullet vs player:
 *      - remove bullet, lose life, possibly game over
 *  - When no aliens remain alive:
 *      - switch to NextWave state (press SPACE to continue)
 */

#include "game/Game.hpp"
#include <iostream>

Game::Game()
    : window(sf::VideoMode((unsigned)cfg::WindowWidth, (unsigned)cfg::WindowHeight), "The Great Sage Tsardock"),
      player(cfg::PlayerStartX, cfg::PlayerStartY, cfg::minX, cfg::maxX),
      alienCooldown(cfg::InitialAlienCooldown),
      alienSpeed(cfg::InitialAlienSpeed),
      aliens(cfg::minX, cfg::maxX, alienSpeed, alienCooldown)
{
    // Optional: limit tearing / stabilize frame pacing.
    // window.setVerticalSyncEnabled(true);

    // Load all assets through a single system (fail fast + centralized ownership).
    resources.loadAll();

    // Background is a static sprite.
    bgSprite.setTexture(resources.background());

    // Player: provide spritesheet + logical frame size for animation slicing.
    player.setTexture(resources.playerSheet(), 41.f, 60.f);

    // AlienFormation owns the grid; it needs the alien spritesheet.
    aliens.setTexture(resources.alienSheet());

    // Define projectile frame rectangles (used by animateAlienBullets()).
    setAlienProjectileTexture(resources.alienProjectileSheet());

    // HUD initialization: receives font and uses window width to center banners.
    hud.init(resources.font(), cfg::WindowWidth);

    // Initial state
    mode  = game::Mode::Running;
    score = 0;
    wave  = 1;

    // Push initial values to HUD (so it’s correct even before first update()).
    hud.setStats(score, player.getLives(), wave);
}

void Game::run() {
    sf::Clock clock;

    while (window.isOpen()) {
        const float dt = clock.restart().asSeconds();

        // Main loop order:
        //  1) process input/events
        //  2) update simulation + rules
        //  3) render scene + HUD
        processEvents(dt);
        update(dt);
        render();
    }
}

void Game::restart() {
    // Clear transient state
    playerBullets.clear();
    alienBullets.clear();

    // Reset progression
    score = 0;
    wave  = 1;

    // Reset player state
    player.reset(cfg::PlayerStartX, cfg::PlayerStartY, cfg::PlayerMaxLives);

    // Reset difficulty
    alienSpeed    = cfg::InitialAlienSpeed;
    alienCooldown = cfg::InitialAlienCooldown;

    // Rebuild formation
    aliens.reset(alienSpeed, alienCooldown);

    // Back to running mode
    mode = game::Mode::Running;

    updateHud();
}

void Game::goForNextWave() {
    // Clear bullets so the next wave starts clean (no lingering projectiles).
    playerBullets.clear();
    alienBullets.clear();

    // Reset player position + allow immediate shooting after wave transition.
    player.setPosition(cfg::PlayerStartX, cfg::PlayerStartY);
    player.resetTimeSinceLastShot();

    // Simple difficulty ramp:
    //  - speed up formation
    //  - shorten alien cooldown (shoot more often)
    alienSpeed    *= 1.1f;
    alienCooldown *= 0.85f;

    wave++;
    aliens.reset(alienSpeed, alienCooldown);

    mode = game::Mode::Running;

    updateHud();
}

void Game::onSpacePressed() {
    // Contextual SPACE:
    //  - NextWave: continue
    //  - GameOver: restart
    //  - Running: try to shoot (player cooldown governs it)
    if (mode == game::Mode::NextWave) {
        goForNextWave();
        return;
    }
    if (mode == game::Mode::GameOver) {
        restart();
        return;
    }
    tryShoot();
}

void Game::tryShoot() {
    // Player owns its own fire-rate limiter (cooldown).
    if (!player.shoot()) return;

    Bullet b;
    b.createBulletAt(
        player.getXCoordinate(),
        player.getTopCoodinate(),
        -cfg::BulletSpeed,             // negative = upward
        resources.playerProjectile()
    );

    playerBullets.push_back(b);

    // Purely visual; no gameplay dependency.
    player.triggerShootAnimation();
}

void Game::processEvents(float dt) {
    (void)dt; // currently unused here; kept for symmetry/future input buffering.

    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) window.close();

        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) window.close();
            if (event.key.code == sf::Keyboard::Space)  onSpacePressed();
        }
    }

    // Continuous movement input (not event-based):
    // We read the current keyboard state each frame for smooth movement.
    const bool left  = sf::Keyboard::isKeyPressed(sf::Keyboard::A);
    const bool right = sf::Keyboard::isKeyPressed(sf::Keyboard::D);

    if (mode == game::Mode::Running) {
        if (left && !right)  player.moveLeft(dt);
        if (right && !left)  player.moveRight(dt);
    }
}

void Game::updateBullets(float dt) {
    // Update bullets and remove those that leave the screen.
    // Index-based erase loop avoids iterator invalidation.

    for (size_t i = 0; i < alienBullets.size(); ) {
        if (!alienBullets[i].update(dt, cfg::WindowHeight))
            alienBullets.erase(alienBullets.begin() + i);
        else
            ++i;
    }

    for (size_t i = 0; i < playerBullets.size(); ) {
        if (!playerBullets[i].update(dt, cfg::WindowHeight))
            playerBullets.erase(playerBullets.begin() + i);
        else
            ++i;
    }
}

void Game::resolvePlayerBulletsVsAliens() {
    // Player bullets vs formation:
    // - if hit: mark alien dead (inside formation), score++, speed ramp, remove bullet
    // - if no aliens remain: switch to NextWave mode

    for (size_t i = 0; i < playerBullets.size(); ) {
        Rect b = playerBullets[i].getBounds();

        if (aliens.hasBeenHit(b)) {
            aliens.decreaseAliveAliens();
            score++;

            // Small intra-wave ramp to increase pressure.
            aliens.increaseHorizontalSpeed();

            // Consume bullet on hit (classic behavior).
            playerBullets.erase(playerBullets.begin() + i);

            if (!aliens.hasAliveAliens()) {
                mode = game::Mode::NextWave;
            }
        } else {
            ++i;
        }
    }
}

void Game::resolveAlienBulletsVsPlayer() {
    // Alien bullets vs player:
    // - On hit: remove bullet, decrement lives
    // - If lives reach 0: GameOver mode

    for (size_t i = 0; i < alienBullets.size(); ) {
        if (player.getBounds().intersects(alienBullets[i].getBounds())) {
            alienBullets.erase(alienBullets.begin() + i);
            player.loseLife();

            if (player.isDead()) {
                mode = game::Mode::GameOver;
            }
        } else {
            ++i;
        }
    }
}

void Game::animateAlienBullets(float dt) {
    // Visual-only animation for alien projectiles:
    // We cycle through texture rects using a small timer per bullet.

    const float t1 = 0.1f;
    const float t2 = 0.2f;
    const float t3 = 0.3f;
    const float t4 = 0.4f;

    for (auto& b : alienBullets) {
        b.alienBulletClock += dt;

        if (b.alienBulletClock < t1) {
            b.sprite.setTextureRect(alienProjectile1);
        } else if (b.alienBulletClock < t2) {
            b.sprite.setTextureRect(alienProjectile2);
        } else if (b.alienBulletClock < t3) {
            b.sprite.setTextureRect(alienProjectile3);
        } else if (b.alienBulletClock < t4) {
            b.sprite.setTextureRect(alienProjectile4);
        } else {
            b.alienBulletClock = 0.f;
        }

        // Ensure sprite stays aligned to the bullet logical position.
        b.sprite.setPosition(b.x, b.y);

        // Keep origin centered even if frames have slightly different bounds.
        const auto bnd = b.sprite.getLocalBounds();
        b.sprite.setOrigin(bnd.left + bnd.width / 2.f,
                           bnd.top  + bnd.height / 2.f);
    }
}

void Game::updateHud() {
    // HUD is derived UI; Game holds the truth.
    hud.setStats(score, player.getLives(), wave);
}

void Game::update(float dt) {
    // When not running, we don't advance simulation.
    // We still refresh HUD to keep stats visible/accurate.
    if (mode != game::Mode::Running) {
        updateHud();
        return;
    }

    // Move formation. If step-down would cross player's vertical space => loss condition.
    if (!aliens.move(dt, player.getTopCoodinate())) {
        mode = game::Mode::GameOver;
        updateHud();
        return;
    }

    // Update bullets first so collisions reflect the latest positions.
    updateBullets(dt);

    // Player animation/cooldowns (movement is handled in processEvents()).
    player.update(dt);

    // Apply collisions.
    resolvePlayerBulletsVsAliens();
    resolveAlienBulletsVsPlayer();

    // Alien shooting:
    // Formation chooses a shooter; Alien owns shoot animation timing + bullet spawn frame.
    if (mode == game::Mode::Running && aliens.hasAliveAliens()) {
        aliens.update(
            alienBullets,
            cfg::BulletSpeed,                 // positive = downward
            dt,
            resources.alienProjectileSheet(), // projectile texture sheet
            alienProjectile1                  // projectile rect (current frame; Game animates it)
        );
    }

    // Visual projectile animation (independent of physics).
    animateAlienBullets(dt);

    updateHud();
}

void Game::setAlienProjectileTexture(const sf::Texture& tex) {
    // Optional sprite (not strictly required if bullets already store texture),
    // but keeping it here documents the slicing of the projectile sheet.
    alienProjectileSprite.setTexture(tex, true);

    // 4-frame projectile spritesheet (horizontal strip).
    alienProjectile1 = sf::IntRect(0,  0, 19, 38);
    alienProjectile2 = sf::IntRect(19, 0, 19, 38);
    alienProjectile3 = sf::IntRect(38, 0, 18, 38);
    alienProjectile4 = sf::IntRect(56, 0, 19, 38);
}

void Game::drawAlienBullets() {
    for (const auto& bullet : alienBullets) {
        window.draw(bullet.sprite);
    }
}

void Game::drawPlayerBullets() {
    for (const auto& bullet : playerBullets) {
        window.draw(bullet.sprite);
    }
}

void Game::render() {
    window.clear();

    // Scene background first
    window.draw(bgSprite);

    // Draw order matters for readability:
    // formation -> bullets -> player -> HUD overlay
    aliens.draw(window);
    drawPlayerBullets();
    player.draw(window);
    drawAlienBullets();

    // HUD last (overlay)
    hud.draw(window, mode);

    window.display();
}
