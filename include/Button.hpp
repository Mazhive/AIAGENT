#pragma once

#include <SFML/Graphics.hpp>

#include <string>

class Button {
public:
  void setup(const sf::Font& font, const std::string& label, sf::Vector2f center, sf::Vector2f size);
  void setEnabled(bool enabled);

  bool hitTest(sf::Vector2f mousePx) const;
  void updateHover(sf::Vector2f mousePx);
  void draw(sf::RenderTarget& target) const;

private:
  sf::RectangleShape m_box;
  sf::Text m_text;
  bool m_hover{false};
  bool m_enabled{true};
};

