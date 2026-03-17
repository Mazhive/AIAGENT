#pragma once

#include <random>

class Random {
public:
  Random();

  int intInclusive(int min, int max);
  float real01();

private:
  std::mt19937 m_rng;
};

