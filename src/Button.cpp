#include "Button.hpp"

void Button::setup(const sf::Font& font, const std::string& label, sf::Vector2f center,
                   sf::Vector2f size) {
  m_box.setSize(size);
  m_box.setOrigin(size.x * 0.5f, size.y * 0.5f);
  m_box.setPosition(center);
  m_box.setOutlineThickness(2.0f);
  m_box.setOutlineColor(sf::Color(70, 75, 92));
  m_box.setFillColor(sf::Color(30, 33, 42));

  m_text.setFont(font);
  m_text.setString(label);
  m_text.setCharacterSize(18);
  m_text.setFillColor(sf::Color(230, 230, 230));
  const auto bounds = m_text.getLocalBounds();
  m_text.setOrigin(bounds.left + bounds.width * 0.5f, bounds.top + bounds.height * 0.5f);
  m_text.setPosition(center.x, center.y - 1.0f);
}

void Button::setEnabled(bool enabled) { m_enabled = enabled; }

bool Button::hitTest(sf::Vector2f mousePx) const { return m_enabled && m_box.getGlobalBounds().contains(mousePx); }

void Button::updateHover(sf::Vector2f mousePx) { m_hover = hitTest(mousePx); }

void Button::draw(sf::RenderTarget& target) const {
  sf::RectangleShape box = m_box;
  sf::Text text = m_text;

  if (!m_enabled) {
    box.setFillColor(sf::Color(26, 28, 34));
    box.setOutlineColor(sf::Color(55, 58, 70));
    text.setFillColor(sf::Color(140, 140, 140));
  } else if (m_hover) {
    box.setFillColor(sf::Color(45, 50, 66));
    box.setOutlineColor(sf::Color(120, 130, 160));
  }

  target.draw(box);
  target.draw(text);
}

