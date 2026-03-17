#pragma once

#include <SFML/Graphics.hpp>

#include "Audio.hpp"
#include "Button.hpp"
#include "Food.hpp"
#include "HighScoreStore.hpp"
#include "Hud.hpp"
#include "Random.hpp"
#include "Snake.hpp"

#include <string>
#include <vector>

class Game {
public:
  Game();

  bool init();
  void run();

private:
  enum class State { Playing, Exploding, GameOver };

  struct Particle {
    sf::Vector2f pos;
    sf::Vector2f vel;
    float lifeSec{0.0f};
  };

  void processEvents();
  void update(float dtSec);
  void render();

  void resetRun();
  void startExplosion(sf::Vector2f centerPx);
  void enterGameOver();

  void spawnNormalFood();
  void spawnSpecialFood();
  sf::Vector2i randomFreeCell();

  bool isInsideGrid(sf::Vector2i cell) const;
  sf::Vector2f cellCenterPx(sf::Vector2i cell) const;

  void rebuildUi();
  void sanitizeName();

private:
  sf::RenderWindow m_window;
  sf::RectangleShape m_background;

  Audio m_audio;
  Random m_rng;
  Snake m_snake;
  Food m_normalFood;
  Food m_specialFood;
  bool m_hasSpecial{false};

  int m_score{0};

  Hud m_hud;
  HighScoreStore m_hiscore;

  float m_moveAccumulator{0.0f};
  float m_movesPerSecond{0.0f};

  State m_state{State::Playing};
  float m_explosionTtlSec{0.0f};
  std::vector<Particle> m_particles;
  bool m_pendingGameOver{false};

  // GameOver UI
  sf::RectangleShape m_overlay;
  sf::Text m_gameOverTitle;
  sf::Text m_nameLabel;
  sf::Text m_nameValue;
  sf::Text m_hint;
  std::string m_nameInput{"PLAYER"};
  bool m_savedThisRun{false};
  Button m_btnSave;
  Button m_btnExit;

  // cached shapes
  sf::CircleShape m_snakeCircle;
  sf::CircleShape m_foodCircle;
};

