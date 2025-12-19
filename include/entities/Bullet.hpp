#pragma once

#include "core/Rect.hpp"

struct Bullet{

    float x, y, width, height, speed, alienBulletClock;

    sf::Sprite sprite;

    //Updates bullet position.
    //Returns false if it is out of the screen.

    bool update(float dt, float windowHeight){

        //Out of screen
        Rect r = getBounds();
        if(r.top > windowHeight || r.bottom < 0.f){
            return false;
        }else{

            y += speed * dt;

            sprite.setPosition(x,y);

            return true;
        }
    }

    //Collision rectangle.
    Rect getBounds() const {
            auto g = sprite.getGlobalBounds();
            Rect r;
            r.left = g.left; r.right = g.left + g.width;
            r.top  = g.top;  r.bottom = g.top + g.height;
            return r;
        }
    
    //Simple function to create a ready bulllet.
    void createBulletAt(const float startX, const float startY, const float bulletSpeed, const sf::Texture& texture){

            x = startX; y = startY; speed = bulletSpeed;
            sprite.setTexture(texture);

            auto b = sprite.getLocalBounds();

            width = b.width;
            height = b.height;
            sprite.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
            sprite.setPosition(x, y);

        }

    void createAlienBulletAt(const float startX, const float startY, const float bulletSpeed, 
        const sf::Texture& texture, const sf::IntRect& RectTexture){

            sprite.setTexture(texture);
            x = startX; y = startY; speed = bulletSpeed;
            sprite.setTextureRect(RectTexture);

            auto b = sprite.getLocalBounds();

            width = b.width;
            height = b.height;
            sprite.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
            sprite.setPosition(x, y);

        }

};