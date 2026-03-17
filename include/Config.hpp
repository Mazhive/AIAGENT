#pragma once

namespace cfg {
inline constexpr int CellSizePx = 24;
inline constexpr int GridWidth = 32;   // 768px
inline constexpr int GridHeight = 24;  // 576px
inline constexpr int TopHudPx = 56;

inline constexpr int WindowWidth = GridWidth * CellSizePx;
inline constexpr int WindowHeight = GridHeight * CellSizePx + TopHudPx;

inline constexpr float BaseMovesPerSecond = 6.0f;
inline constexpr float SpeedPerScore = 0.18f;
inline constexpr float MaxMovesPerSecond = 18.0f;

inline constexpr int NormalFoodScore = 1;
inline constexpr int SpecialFoodScore = 5;
inline constexpr float SpecialFoodLifetimeSec = 6.0f;
inline constexpr float SpecialFoodBlinkHz = 8.0f;

inline constexpr float ExplosionDurationSec = 0.55f;

inline constexpr char FontPath[] = "assets/PressStart2P-Regular.ttf";
inline constexpr char HighScoreFile[] = "highscore.txt";
} // namespace cfg

