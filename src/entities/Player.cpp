#include "entities/Player.hpp"

/**
 * @brief Constructs a Player with default gameplay parameters.
 *
 * Initializes position, movement boundaries (based on window width), and shooting cooldown state.
 * Note: the sprite texture is assigned later via setTexture(), so this constructor focuses on gameplay state.
 */
Player::Player(float startX, float startY, float minX, float maxX)
    : lives(3),
      x(startX),
      y(startY),
      width(52.f),
      height(52.f),
      speed(300.f),
      cooldown(0.3f),
      timeSinceLastShot(cooldown),
      minCoordinateX(minX),
      maxCoordinateX(maxX)
      {}

/**
 * @brief Resets the player to a fresh state (used on game restart).
 *
 * Restores lives, movement/shooting parameters, and places the player back at the start position.
 */
void Player::reset(float startX, float startY, int maxLives) {
    lives = maxLives;
    width = 52.f;
    height = 52.f;
    speed = 300.f;
    cooldown = 0.3f;
    timeSinceLastShot = cooldown;
    setPosition(startX, startY);
}

/**
 * @brief Moves the player left with frame-rate independent motion.
 *
 * Uses dt-based displacement and clamps movement within horizontal bounds.
 * Updates the sprite position to match the gameplay coordinates (x, y).
 */
void Player::moveLeft(float dt) {
    const float newX = x - speed * dt;
    if (newX - width / 2.f > minCoordinateX) {
        x = newX;
        sprite.setPosition(x, y);
    }
}

/**
 * @brief Moves the player right with frame-rate independent motion.
 *
 * Uses dt-based displacement and clamps movement within horizontal bounds.
 * Updates the sprite position to match the gameplay coordinates (x, y).
 */
void Player::moveRight(float dt) {
    const float newX = x + speed * dt;
    if (newX + width / 2.f < maxCoordinateX){
        x = newX;
        sprite.setPosition(x, y);
    }
}

/**
 * @brief Attempts to fire a shot based on the current cooldown.
 *
 * @return true if the player is allowed to shoot now (cooldown elapsed); false otherwise.
 *
 * If successful, resets the internal cooldown timer to 0.
 * The Game layer is responsible for actually spawning the bullet entity.
 */
bool Player::shoot() {
    if (cooldown > timeSinceLastShot) return false;
    timeSinceLastShot = 0.f;
    return true;
}

/**
 * @brief Updates player timers.
 *
 * Used to advance the shooting cooldown timer using dt (seconds),
 * and to restore player to idle sprite if animation has ended.
 */
void Player::update(float dt) {
    timeSinceLastShot += dt;

    if(shootingAnim){
        shootingAnimTimer += dt;
        if(shootingAnimTimer > shootingAnimDuration){
        shootingAnim = false;
        sprite.setTextureRect(idleRect);
        }
    }
}

/**
 * @brief Returns the player's collision bounds.
 *
 * Uses the sprite's global bounds (AABB) so the hitbox matches the sprite's current
 * position and scale.
 */
Rect Player::getBounds() const {
    auto g = sprite.getGlobalBounds();
    Rect r;
    r.left   = g.left;
    r.right  = g.left + g.width;
    r.top    = g.top;
    r.bottom = g.top + g.height;
    return r;
}

/**
 * @brief Returns the player's current X coordinate (center-based).
 *
 * Used by Game to position bullets or query player state.
 */
float Player::getXCoordinate() const {
    return x; 
}

/**
 * @brief Returns the Y coordinate of the top edge of the player.
 *
 * This is convenient for spawning bullets slightly above the player.
 */
float Player::getTopCoodinate() const {
    Rect r = getBounds();
    return r.top;
}

/**
 * @brief Returns the current number of lives.
 */
int Player::getLives() const {
    return lives;
}

/**
 * @brief Returns whether the player has no lives left.
 */
bool Player::isDead() const {
    return lives <= 0;
}

/**
 * @brief Decrements the player's lives by one.
 *
 * Game controls when this is called (e.g., after a bullet collision).
 */
void Player::loseLife() {
    lives--;
}

/**
 * @brief Sets the player's position (gameplay coordinates + sprite position).
 *
 * Keeps gameplay state (x, y) and visual state (sprite position) in sync.
 */
void Player::setPosition(float X, float Y) {
    x = X;
    y = Y;
    sprite.setPosition(x, y);
}

/**
 * @brief Resets the shooting timer so the player can shoot immediately.
 *
 * Used when transitioning to a new wave to avoid “locked” shooting at wave start.
 */
void Player::resetTimeSinceLastShot() {
    timeSinceLastShot = cooldown;
}

/**
 * @brief Assigns the player texture and configures sprite transform.
 *
 * Sets:
 *  - the texture reference in the sprite
 *  - origin to the center (so positioning uses the sprite's center)
 *  - scale so the sprite matches the desired logical size (desiredW x desiredH)
 */
void Player::setTexture(const sf::Texture& tex, float desiredW, float desiredH) {

    sprite.setTexture(tex, true);

    const int frameW = 40.f;
    const int frameH = 60.f;

    idleRect  = sf::IntRect(0, 0, frameW, frameH);
    shootRect = sf::IntRect(frameW, 0, frameW, frameH);

    sprite.setTextureRect(idleRect);

    auto b = sprite.getLocalBounds();

    sprite.setOrigin(b.width / 2.f, b.height / 2.f);
    sprite.setScale(desiredW / b.width, desiredH / b.height);
    sprite.setPosition(x, y);

}

/**
 * @brief Assigns the player shooting texture.
 *
 * Sets:
 *  - the texture reference in the sprite
 */
void Player::triggerShootAnimation() {
    shootingAnim = true;
    shootingAnimTimer = 0.f;
    sprite.setTextureRect(shootRect);
}

/**
 * @brief Renders the player sprite.
 */
void Player::draw(sf::RenderWindow& window) const {
    if(timeSinceLastShot)
    window.draw(sprite);
}
