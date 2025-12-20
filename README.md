# The Great Sage Tsardock  
*A Space Invaders-inspired C++ / SMFL learning project - with a wizard-themed twist*

This repository is my first GitHub project and was built primarily as a learning journey.

It started as a classic **Space Invaders** clone to practice core game programming fundamentals in C++ (game loop, input, collisions, timing, and basic architecture). Over time, inspired by my brother, the project gained its own identity — the theme shifted toward a **wizard/magic** vibe — while the **gameplay and rules remain Space-Invaders-like**: waves, a marching formation, bullets, lives, and score.

---

## Gameplay
- Classic **formation movement** (side-to-side → edge hit → step-down → direction flip)
- **Waves** with progressive difficulty (formation speed increases and shooting gets more frequent)
- **Shooter candidates**: only the lowest living enemy in each column can shoot (classic behavior)
- **HUD**: score, lives, wave + state banners (Game Over / Next Wave)
- **Pixel-art friendly rendering**: pixel snapping + consistent sprite origins

---

## Controls
- **A / D** — Move
- **SPACE** — Shoot (or restart / next wave depending on state)
- **ESC** — Quit

---

## Project focus (what I practiced here)
- **dt-driven simulation** for stable behavior across frame rates
- A clean separation of responsibilities:
  - `Game` as the composition root (rules + orchestration)
  - `AlienFormation` for formation logic + shooter selection
  - `Resources` for centralized asset loading
  - `Hud` as a pure UI layer (no gameplay state inside)
- Safe container iteration patterns (erase while iterating)
- Building a small project without frameworks (manual build via VSCode tasks)
```
---

## Folder structure
include/ # headers
src/ # .cpp source files
assets/ # sprites + fonts
.vscode/ # tasks/launch (optional, for easy build/run)
bin/ # build output (ignored by git)

---

## Build (Windows / MinGW g++)
This project is compiled using **g++ (MinGW)** and **SFML**.

Example command (adjust paths as needed):

```bash
g++ -std=c++17 -g -O0 -Wall -Wextra ^
  -I include -IC:/SFML-2.6.1/include ^
  src/main.cpp ^
  src/game/Game.cpp ^
  src/game/AlienFormation.cpp ^
  src/entities/Player.cpp ^
  src/ui/Hud.cpp ^
  src/systems/Resources.cpp ^
  -LC:/SFML-2.6.1/lib ^
  -lsfml-graphics -lsfml-window -lsfml-system ^
  -o bin/space_invaders.exe


---


Sprites and background were generated with AI tools and are included for educational/demo purposes (non-commercial).


---


License

MIT


---

