#pragma once

#include <SFML/System/Vector2.hpp>

enum class FoodType { Normal, Special };

struct Food {
  FoodType type{FoodType::Normal};
  sf::Vector2i cell{0, 0};
  float ttlSec{0.0f};
};

