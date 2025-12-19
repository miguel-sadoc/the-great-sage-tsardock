#pragma once

#include "core/Rect.hpp"
#include <SFML/Graphics.hpp>


class Player{

    int lives;

    float x, y;
    float width, height;
    float speed;

    float cooldown;
    float timeSinceLastShot;
    float minCoordinateX;
    float maxCoordinateX;

    sf::Sprite sprite;

    sf::IntRect idleRect{};
    sf::IntRect shootRect{};

    bool shootingAnim{false};
    float shootingAnimTimer{0.f};
    float shootingAnimDuration{0.2f};
    

    public:

        Player(float startX, float startY, float minX, float maxX);
        
        //Resets player to original settings.
        void reset(float startX, float startY, int maxLives);

        //Movements.
        void moveLeft(float dt);
        void moveRight(float dt);
        
        //Retunrs false if cant shoot.
        bool shoot();

        //Updates time since last shot.
        void update(float dt);

        //Returns collision rectangle.
        Rect getBounds() const;

        //Returns player's x coordinate.
        float getXCoordinate() const;

        //Returns player's top coordinate.
        float getTopCoodinate() const;

        //Returns player's lives.
        int getLives() const;

        //Returns false if player is alive.
        bool isDead() const;

        //Decreases one life from player.
        void loseLife();
        
        //Changes player's position.
        void setPosition(const float X, const float Y);

        //Resets time since last shot to cooldown.
        void resetTimeSinceLastShot();

        //Sets the player's sprite.
        void setTexture(const sf::Texture& tex, float desiredW, float desiredH);

        void triggerShootAnimation();
        
        //Draws player.
        void draw(sf::RenderWindow& window) const;

};