#pragma once

#include <string>

struct HighScoreEntry {
  int score{0};
  std::string name{"---"};
};

class HighScoreStore {
public:
  bool load();
  bool save() const;

  const HighScoreEntry& entry() const;
  void setIfBetter(int score, const std::string& name);

private:
  HighScoreEntry m_entry;
};

