#include "Snake.hpp"

#include <cstddef>

void Snake::reset(sf::Vector2i startCell) {
  m_cells.clear();
  m_cells.push_back(startCell);
  m_cells.push_back({startCell.x - 1, startCell.y});
  m_cells.push_back({startCell.x - 2, startCell.y});
  m_dir = Dir::Right;
  m_nextDir = Dir::Right;
}

bool Snake::isOpposite(Dir a, Dir b) {
  return (a == Dir::Up && b == Dir::Down) || (a == Dir::Down && b == Dir::Up) ||
         (a == Dir::Left && b == Dir::Right) || (a == Dir::Right && b == Dir::Left);
}

bool Snake::trySetDir(Dir d) {
  if (isOpposite(d, m_nextDir)) return false;
  m_nextDir = d;
  return true;
}

bool Snake::wouldReverse(Dir requested) const { return isOpposite(requested, m_nextDir); }

Snake::Dir Snake::dir() const { return m_dir; }
Snake::Dir Snake::nextDir() const { return m_nextDir; }

const std::deque<sf::Vector2i>& Snake::cells() const { return m_cells; }
sf::Vector2i Snake::head() const { return m_cells.front(); }

bool Snake::occupies(sf::Vector2i cell) const {
  for (const auto& c : m_cells) {
    if (c == cell) return true;
  }
  return false;
}

void Snake::step(bool grow) {
  m_dir = m_nextDir;
  auto h = head();
  switch (m_dir) {
    case Dir::Up: h.y -= 1; break;
    case Dir::Down: h.y += 1; break;
    case Dir::Left: h.x -= 1; break;
    case Dir::Right: h.x += 1; break;
  }

  m_cells.push_front(h);
  if (!grow) m_cells.pop_back();
}

bool Snake::selfCollision() const {
  if (m_cells.size() < 4) return false;
  const auto h = head();
  for (std::size_t i = 1; i < m_cells.size(); ++i) {
    if (m_cells[i] == h) return true;
  }
  return false;
}

