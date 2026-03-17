#pragma once

#include <SFML/System/Vector2.hpp>

#include <deque>

class Snake {
public:
  enum class Dir { Up, Down, Left, Right };

  void reset(sf::Vector2i startCell);

  bool trySetDir(Dir d);
  bool wouldReverse(Dir requested) const;
  Dir dir() const;
  Dir nextDir() const;

  void step(bool grow);

  const std::deque<sf::Vector2i>& cells() const;
  sf::Vector2i head() const;
  bool occupies(sf::Vector2i cell) const;
  bool selfCollision() const;

private:
  static bool isOpposite(Dir a, Dir b);

  std::deque<sf::Vector2i> m_cells;
  Dir m_dir{Dir::Right};
  Dir m_nextDir{Dir::Right};
};

