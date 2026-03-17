#include "Game.hpp"

#include <iostream>

int main() {
  Game game;
  if (!game.init()) {
    std::cerr << "Failed to initialize game. Missing assets?\n";
    return 1;
  }
  game.run();
  return 0;
}

