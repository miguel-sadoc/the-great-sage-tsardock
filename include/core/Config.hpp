#pragma once

/**
 * @file Config.hpp
 * @brief Centralized gameplay + layout constants.
 *
 * Portfolio notes:
 *  - This header is intentionally "data-only": no game logic, just tunable parameters.
 *  - Keeping numbers here makes balancing and layout iteration fast (single source of truth).
 *  - Values are expressed in pixels (screen/world space) and seconds.
 */

#include <cmath> // std::round

namespace cfg {

    // -------------------------------------------------------------------------
    // Window / World
    // -------------------------------------------------------------------------

    /// Target window resolution (pixels). Used to create the SFML RenderWindow.
    inline constexpr float WindowWidth  = 1536.f;
    inline constexpr float WindowHeight = 925.f;

    /**
     * Horizontal play area clamp for the player.
     * These are world/screen X boundaries that keep the player inside the corridor/arena.
     *
     * Tip:
     *  - If you change the background or corridor width, update these values accordingly.
     */
    inline constexpr float minX = 478.f;
    inline constexpr float maxX = 1056.f;

    // -------------------------------------------------------------------------
    // Bullets
    // -------------------------------------------------------------------------

    /// Logical bullet size (used for collision / spawn offsets when needed).
    inline constexpr float BulletWidth  = 3.f;
    inline constexpr float BulletHeight = 10.f;

    /// Bullet vertical speed (pixels/second). Player uses negative, aliens use positive.
    inline constexpr float BulletSpeed  = 370.f;

    // -------------------------------------------------------------------------
    // Player
    // -------------------------------------------------------------------------

    /// Starting lives per run.
    inline constexpr int   PlayerMaxLives = 3;

    /// Player spawn position (center coordinates).
    inline constexpr float PlayerStartX = 768.f;
    inline constexpr float PlayerStartY = 750.f;

    // -------------------------------------------------------------------------
    // Aliens (difficulty + formation layout)
    // -------------------------------------------------------------------------

    /**
     * Initial firing cooldown (seconds) used for alien shooting pacing.
     * Game can scale this down per wave for difficulty ramp.
     */
    inline constexpr float InitialAlienCooldown = 3.f;

    /**
     * Initial formation horizontal speed (pixels/second).
     * Game can scale this up per wave for difficulty ramp.
     */
    inline constexpr float InitialAlienSpeed    = 20.f;

    /// Discrete step-down distance (pixels) applied when the formation hits a side boundary.
    /// This is intentionally NOT dt-based: it behaves like a classic arcade "event" step.
    inline constexpr float AliensVerticalStep = 10.f;

    /// Formation grid dimensions.
    inline constexpr int   AliensRows = 5;
    inline constexpr int   AliensCols = 8;

    /**
     * Formation layout anchor and spacing.
     * Note: Alien positions are treated as centered coordinates (sprite origin centered).
     */
    inline constexpr float AliensStartX = 536.f;
    inline constexpr float AliensStartY = 170.f;
    inline constexpr float AliensHorizontalGap = 25.f;
    inline constexpr float AliensVerticalGap   = 20.f;

    /// Alien sprite logical size (kept here for consistency across layout + offsets).
    inline constexpr float AlienWidth  = 41.f;
    inline constexpr float AlienHeight = 35.f;

    // -------------------------------------------------------------------------
    // Pixel-art rendering helper
    // -------------------------------------------------------------------------

    /**
     * @brief Pixel snapping helper for crisp pixel-art rendering.
     *
     * Rationale:
     *  - Pixel art looks best when sprites land on whole-pixel coordinates.
     *  - Sub-pixel positions can cause shimmering/blur due to texture filtering.
     *
     * Usage:
     *  - sprite.setPosition(cfg::PixelSnap(x), cfg::PixelSnap(y));
     *
     * If you ever change your policy (floor/ceil/custom grid), update it here.
     */
    inline float PixelSnap(float v) {
        return std::round(v);
    }

} // namespace cfg
