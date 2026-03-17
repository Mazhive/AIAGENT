#pragma once

#include <SFML/Graphics.hpp>

#include <string>

class Hud {
public:
  bool load();
  void setScore(int score, int highScore, const std::string& highName);
  void draw(sf::RenderTarget& target) const;

private:
  sf::Font m_font;
  sf::Text m_text;
};

