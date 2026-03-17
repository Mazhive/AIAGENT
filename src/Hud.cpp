#include "Hud.hpp"

#include "Config.hpp"

#include <sstream>

bool Hud::load() {
  if (!m_font.loadFromFile(cfg::FontPath)) return false;
  m_text.setFont(m_font);
  m_text.setCharacterSize(18);
  m_text.setFillColor(sf::Color(230, 230, 230));
  m_text.setPosition(12.0f, 14.0f);
  setScore(0, 0, "---");
  return true;
}

void Hud::setScore(int score, int highScore, const std::string& highName) {
  std::ostringstream oss;
  oss << "SCORE " << score << "   HI " << highName << " " << highScore;
  m_text.setString(oss.str());
}

void Hud::draw(sf::RenderTarget& target) const { target.draw(m_text); }

