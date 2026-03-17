#include "Random.hpp"

#include <chrono>

Random::Random() {
  const auto seed = static_cast<unsigned>(
      std::chrono::high_resolution_clock::now().time_since_epoch().count());
  m_rng.seed(seed);
}

int Random::intInclusive(int min, int max) {
  std::uniform_int_distribution<int> dist(min, max);
  return dist(m_rng);
}

float Random::real01() {
  std::uniform_real_distribution<float> dist(0.0f, 1.0f);
  return dist(m_rng);
}

