#include "Game.hpp"

#include "Config.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace {
sf::Color lerpColor(const sf::Color& a, const sf::Color& b, float t) {
  t = std::clamp(t, 0.0f, 1.0f);
  const auto lerp = [&](sf::Uint8 x, sf::Uint8 y) -> sf::Uint8 {
    return static_cast<sf::Uint8>(std::lround(x + (y - x) * t));
  };
  return sf::Color(lerp(a.r, b.r), lerp(a.g, b.g), lerp(a.b, b.b), lerp(a.a, b.a));
}

sf::Vector2f centerOfWindow() {
  return {cfg::WindowWidth * 0.5f, cfg::WindowHeight * 0.5f};
}
} // namespace

Game::Game()
    : m_window(sf::VideoMode(cfg::WindowWidth, cfg::WindowHeight), "Advanced Snake (SFML)") {
  m_window.setVerticalSyncEnabled(true);
  m_window.setKeyRepeatEnabled(false);
}

bool Game::init() {
  if (!m_hud.load()) return false;

  (void)m_hiscore.load();
  (void)m_audio.init();

  m_background.setSize(
      sf::Vector2f(static_cast<float>(cfg::WindowWidth), static_cast<float>(cfg::WindowHeight)));
  m_background.setFillColor(sf::Color(20, 22, 28));

  const float radius = (cfg::CellSizePx * 0.5f) - 2.0f;
  m_snakeCircle.setRadius(radius);
  m_snakeCircle.setOrigin(radius, radius);
  m_snakeCircle.setPointCount(20);

  m_foodCircle.setRadius(radius - 2.0f);
  m_foodCircle.setOrigin(radius - 2.0f, radius - 2.0f);
  m_foodCircle.setPointCount(20);

  rebuildUi();
  resetRun();
  return true;
}

void Game::rebuildUi() {
  // Overlay
  m_overlay.setSize(sf::Vector2f(static_cast<float>(cfg::WindowWidth), static_cast<float>(cfg::WindowHeight)));
  m_overlay.setFillColor(sf::Color(0, 0, 0, 160));

  // We reuse the HUD font via rendering texts with the same font instance.
  // HUD keeps its own font; for UI we load another font from the same path.
  static sf::Font uiFont;
  static bool loaded = false;
  if (!loaded) {
    uiFont.loadFromFile(cfg::FontPath);
    loaded = true;
  }

  m_gameOverTitle.setFont(uiFont);
  m_gameOverTitle.setCharacterSize(28);
  m_gameOverTitle.setFillColor(sf::Color(240, 240, 240));
  m_gameOverTitle.setString("GAME OVER");
  auto b = m_gameOverTitle.getLocalBounds();
  m_gameOverTitle.setOrigin(b.left + b.width * 0.5f, b.top + b.height * 0.5f);
  m_gameOverTitle.setPosition(centerOfWindow().x, centerOfWindow().y - 130.0f);

  m_nameLabel.setFont(uiFont);
  m_nameLabel.setCharacterSize(18);
  m_nameLabel.setFillColor(sf::Color(200, 200, 200));
  m_nameLabel.setString("NAME:");
  b = m_nameLabel.getLocalBounds();
  m_nameLabel.setOrigin(b.left + b.width, b.top + b.height * 0.5f);
  m_nameLabel.setPosition(centerOfWindow().x - 60.0f, centerOfWindow().y - 60.0f);

  m_nameValue.setFont(uiFont);
  m_nameValue.setCharacterSize(18);
  m_nameValue.setFillColor(sf::Color(255, 255, 255));
  m_nameValue.setPosition(centerOfWindow().x - 40.0f, centerOfWindow().y - 74.0f);

  m_hint.setFont(uiFont);
  m_hint.setCharacterSize(14);
  m_hint.setFillColor(sf::Color(180, 180, 180));
  m_hint.setString("TYPE YOUR NAME, CLICK SAVE.  R TO RESTART");
  b = m_hint.getLocalBounds();
  m_hint.setOrigin(b.left + b.width * 0.5f, b.top + b.height * 0.5f);
  m_hint.setPosition(centerOfWindow().x, centerOfWindow().y + 90.0f);

  m_btnSave.setup(uiFont, "SAVE HISCORE", {centerOfWindow().x - 120.0f, centerOfWindow().y + 10.0f},
                  {220.0f, 54.0f});
  m_btnExit.setup(uiFont, "EXIT", {centerOfWindow().x + 160.0f, centerOfWindow().y + 10.0f},
                  {160.0f, 54.0f});
}

void Game::run() {
  sf::Clock clock;
  while (m_window.isOpen()) {
    processEvents();
    const float dt = clock.restart().asSeconds();
    update(dt);
    render();
  }
}

void Game::resetRun() {
  m_score = 0;
  m_moveAccumulator = 0.0f;
  m_movesPerSecond = cfg::BaseMovesPerSecond;
  m_state = State::Playing;
  m_explosionTtlSec = 0.0f;
  m_particles.clear();
  m_pendingGameOver = false;
  m_savedThisRun = false;

  const sf::Vector2i start{cfg::GridWidth / 2, cfg::GridHeight / 2};
  m_snake.reset(start);

  spawnNormalFood();
  m_hasSpecial = false;

  m_hud.setScore(m_score, m_hiscore.entry().score, m_hiscore.entry().name);
}

void Game::sanitizeName() {
  // Keep only a simple safe charset to avoid weird file contents / rendering issues.
  std::string out;
  out.reserve(m_nameInput.size());
  for (unsigned char ch : m_nameInput) {
    if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') ||
        ch == '_' || ch == '-' || ch == ' ') {
      out.push_back(static_cast<char>(ch));
    }
    if (out.size() >= 14) break;
  }
  out.erase(out.begin(), std::find_if(out.begin(), out.end(), [](char c) { return c != ' '; }));
  while (!out.empty() && out.back() == ' ') out.pop_back();
  if (out.empty()) out = "PLAYER";
  m_nameInput = out;
}

void Game::startExplosion(sf::Vector2f centerPx) {
  m_state = State::Exploding;
  m_explosionTtlSec = cfg::ExplosionDurationSec;
  m_moveAccumulator = 0.0f;
  m_pendingGameOver = true;
  m_audio.playExplosion();

  m_particles.clear();
  m_particles.reserve(56);
  const float baseSpeed = 240.0f;

  for (int i = 0; i < 56; ++i) {
    const float a = (static_cast<float>(i) / 56.0f) * 6.2831853f;
    const float jitter = (m_rng.real01() - 0.5f) * 0.55f;
    const float ang = a + jitter;
    const float sp = baseSpeed * (0.6f + 0.9f * m_rng.real01());
    Particle p;
    p.pos = centerPx;
    p.vel = {std::cos(ang) * sp, std::sin(ang) * sp};
    p.lifeSec = m_explosionTtlSec * (0.65f + 0.35f * m_rng.real01());
    m_particles.push_back(p);
  }
}

void Game::enterGameOver() {
  m_state = State::GameOver;
  sanitizeName();
  m_nameValue.setString(m_nameInput + "_");
  m_btnSave.setEnabled(!m_savedThisRun && m_score > m_hiscore.entry().score);
}

void Game::processEvents() {
  sf::Event ev{};
  while (m_window.pollEvent(ev)) {
    if (ev.type == sf::Event::Closed) {
      m_window.close();
      continue;
    }

    if (ev.type == sf::Event::KeyPressed) {
      if (ev.key.code == sf::Keyboard::Escape) {
        m_window.close();
        continue;
      }
      if (ev.key.code == sf::Keyboard::R) {
        resetRun();
        continue;
      }
    }

    if (m_state == State::Playing) {
      if (ev.type == sf::Event::KeyPressed) {
        Snake::Dir requested = m_snake.nextDir();
        bool isDirKey = true;
        if (ev.key.code == sf::Keyboard::Up || ev.key.code == sf::Keyboard::W) {
          requested = Snake::Dir::Up;
        } else if (ev.key.code == sf::Keyboard::Down || ev.key.code == sf::Keyboard::S) {
          requested = Snake::Dir::Down;
        } else if (ev.key.code == sf::Keyboard::Left || ev.key.code == sf::Keyboard::A) {
          requested = Snake::Dir::Left;
        } else if (ev.key.code == sf::Keyboard::Right || ev.key.code == sf::Keyboard::D) {
          requested = Snake::Dir::Right;
        } else {
          isDirKey = false;
        }

        if (isDirKey) {
          switch (requested) {
            case Snake::Dir::Up: m_audio.playDirUp(); break;
            case Snake::Dir::Down: m_audio.playDirDown(); break;
            case Snake::Dir::Left: m_audio.playDirLeft(); break;
            case Snake::Dir::Right: m_audio.playDirRight(); break;
          }

          if (m_snake.wouldReverse(requested)) {
            startExplosion(cellCenterPx(m_snake.head()));
          } else {
            m_snake.trySetDir(requested);
          }
        }
      }
    } else if (m_state == State::GameOver) {
      if (ev.type == sf::Event::TextEntered) {
        const sf::Uint32 u = ev.text.unicode;
        if (u == 8) { // backspace
          if (!m_nameInput.empty()) m_nameInput.pop_back();
        } else if (u == 13) {
          // enter: no-op (save via button)
        } else if (u >= 32 && u < 127) {
          m_nameInput.push_back(static_cast<char>(u));
        }
        sanitizeName();
        m_nameValue.setString(m_nameInput + "_");
      } else if (ev.type == sf::Event::MouseButtonPressed && ev.mouseButton.button == sf::Mouse::Left) {
        const sf::Vector2f mousePx = m_window.mapPixelToCoords({ev.mouseButton.x, ev.mouseButton.y});
        if (m_btnExit.hitTest(mousePx)) {
          m_window.close();
        } else if (m_btnSave.hitTest(mousePx)) {
          sanitizeName();
          m_hiscore.setIfBetter(m_score, m_nameInput);
          (void)m_hiscore.save();
          m_savedThisRun = true;
          m_btnSave.setEnabled(false);
          m_hud.setScore(m_score, m_hiscore.entry().score, m_hiscore.entry().name);
        }
      }
    }
  }
}

bool Game::isInsideGrid(sf::Vector2i cell) const {
  return cell.x >= 0 && cell.x < cfg::GridWidth && cell.y >= 0 && cell.y < cfg::GridHeight;
}

sf::Vector2f Game::cellCenterPx(sf::Vector2i cell) const {
  const float x = (cell.x + 0.5f) * cfg::CellSizePx;
  const float y = cfg::TopHudPx + (cell.y + 0.5f) * cfg::CellSizePx;
  return {x, y};
}

sf::Vector2i Game::randomFreeCell() {
  for (int i = 0; i < 256; ++i) {
    sf::Vector2i c{m_rng.intInclusive(0, cfg::GridWidth - 1),
                   m_rng.intInclusive(0, cfg::GridHeight - 1)};
    if (m_snake.occupies(c)) continue;
    if (c == m_normalFood.cell) continue;
    if (m_hasSpecial && c == m_specialFood.cell) continue;
    return c;
  }

  for (int y = 0; y < cfg::GridHeight; ++y) {
    for (int x = 0; x < cfg::GridWidth; ++x) {
      sf::Vector2i c{x, y};
      if (!m_snake.occupies(c) && c != m_normalFood.cell &&
          (!m_hasSpecial || c != m_specialFood.cell)) {
        return c;
      }
    }
  }
  return {0, 0};
}

void Game::spawnNormalFood() {
  m_normalFood.type = FoodType::Normal;
  m_normalFood.cell = randomFreeCell();
  m_normalFood.ttlSec = 0.0f;
}

void Game::spawnSpecialFood() {
  m_specialFood.type = FoodType::Special;
  m_specialFood.cell = randomFreeCell();
  m_specialFood.ttlSec = cfg::SpecialFoodLifetimeSec;
  m_hasSpecial = true;
}

void Game::update(float dtSec) {
  if (m_state == State::Exploding) {
    m_explosionTtlSec -= dtSec;
    for (auto& p : m_particles) {
      p.lifeSec -= dtSec;
      if (p.lifeSec <= 0.0f) continue;
      p.pos += p.vel * dtSec;
      p.vel *= std::pow(0.02f, dtSec);
    }
    if (m_explosionTtlSec <= 0.0f) {
      if (m_pendingGameOver) enterGameOver();
      else resetRun();
    }
    return;
  }

  if (m_state != State::Playing) return;

  // Special food timer + despawn.
  if (m_hasSpecial) {
    m_specialFood.ttlSec -= dtSec;
    if (m_specialFood.ttlSec <= 0.0f) m_hasSpecial = false;
  } else {
    const float chancePerSec = 0.04f + 0.002f * static_cast<float>(m_score);
    if (m_rng.real01() < chancePerSec * dtSec) spawnSpecialFood();
  }

  // Movement tick based on score.
  m_movesPerSecond =
      std::min(cfg::MaxMovesPerSecond, cfg::BaseMovesPerSecond + cfg::SpeedPerScore * m_score);
  const float stepEvery = 1.0f / m_movesPerSecond;
  m_moveAccumulator += dtSec;

  while (m_moveAccumulator >= stepEvery) {
    m_moveAccumulator -= stepEvery;

    m_audio.playWalk();

    sf::Vector2i next = m_snake.head();
    switch (m_snake.nextDir()) {
      case Snake::Dir::Up: next.y -= 1; break;
      case Snake::Dir::Down: next.y += 1; break;
      case Snake::Dir::Left: next.x -= 1; break;
      case Snake::Dir::Right: next.x += 1; break;
    }

    if (!isInsideGrid(next)) {
      startExplosion(cellCenterPx(m_snake.head()));
      return;
    }

    bool grow = false;
    if (next == m_normalFood.cell) {
      grow = true;
      m_score += cfg::NormalFoodScore;
      spawnNormalFood();
    } else if (m_hasSpecial && next == m_specialFood.cell) {
      grow = true;
      m_score += cfg::SpecialFoodScore;
      m_hasSpecial = false;
    }

    m_snake.step(grow);

    if (m_snake.selfCollision()) {
      startExplosion(cellCenterPx(m_snake.head()));
      return;
    }

    m_hud.setScore(m_score, m_hiscore.entry().score, m_hiscore.entry().name);
  }
}

void Game::render() {
  m_window.clear();
  m_window.draw(m_background);

  // Subtle grid background (dots).
  sf::CircleShape dot(1.0f);
  dot.setFillColor(sf::Color(45, 48, 58));
  for (int y = 0; y < cfg::GridHeight; ++y) {
    for (int x = 0; x < cfg::GridWidth; ++x) {
      const auto p = cellCenterPx({x, y});
      dot.setPosition(p.x - 1.0f, p.y - 1.0f);
      m_window.draw(dot);
    }
  }

  // Foods
  m_foodCircle.setFillColor(sf::Color(235, 80, 80));
  m_foodCircle.setPosition(cellCenterPx(m_normalFood.cell));
  m_window.draw(m_foodCircle);

  if (m_hasSpecial) {
    const float t = std::fmod(std::max(0.0f, m_specialFood.ttlSec) * cfg::SpecialFoodBlinkHz, 1.0f);
    const bool on = t > 0.5f;
    const auto col = on ? sf::Color(80, 220, 255) : sf::Color(80, 220, 255, 40);
    m_foodCircle.setFillColor(col);
    m_foodCircle.setPosition(cellCenterPx(m_specialFood.cell));
    m_window.draw(m_foodCircle);
  }

  // Snake (circles)
  const sf::Color headCol(120, 255, 140);
  const sf::Color bodyCol(40, 200, 90);
  const auto& cells = m_snake.cells();
  for (std::size_t i = 0; i < cells.size(); ++i) {
    const float t = cells.size() <= 1 ? 0.0f : static_cast<float>(i) / static_cast<float>(cells.size() - 1);
    m_snakeCircle.setFillColor(lerpColor(headCol, bodyCol, t));
    m_snakeCircle.setPosition(cellCenterPx(cells[i]));
    m_window.draw(m_snakeCircle);
  }

  // Explosion particles
  if (m_state == State::Exploding) {
    sf::CircleShape spark(3.0f);
    spark.setOrigin(3.0f, 3.0f);
    spark.setPointCount(12);
    for (const auto& p : m_particles) {
      if (p.lifeSec <= 0.0f) continue;
      const float alpha = std::clamp(p.lifeSec / cfg::ExplosionDurationSec, 0.0f, 1.0f);
      spark.setFillColor(sf::Color(255, 220, 120, static_cast<sf::Uint8>(alpha * 255)));
      spark.setPosition(p.pos);
      m_window.draw(spark);
    }
  }

  m_hud.draw(m_window);

  if (m_state == State::GameOver) {
    const sf::Vector2f mousePx = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window));
    m_btnSave.updateHover(mousePx);
    m_btnExit.updateHover(mousePx);

    m_window.draw(m_overlay);
    m_window.draw(m_gameOverTitle);
    m_window.draw(m_nameLabel);
    m_window.draw(m_nameValue);
    m_btnSave.draw(m_window);
    m_btnExit.draw(m_window);
    m_window.draw(m_hint);
  }

  m_window.display();
}

