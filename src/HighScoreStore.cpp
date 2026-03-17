#include "HighScoreStore.hpp"

#include "Config.hpp"

#include <fstream>
#include <sstream>

namespace {
std::string trim(const std::string& s) {
  size_t a = 0;
  while (a < s.size() && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
  size_t b = s.size();
  while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
  return s.substr(a, b - a);
}
} // namespace

bool HighScoreStore::load() {
  std::ifstream in(cfg::HighScoreFile);
  if (!in) return false;

  // Format: <score>\n<name>\n
  std::string scoreLine;
  std::string nameLine;
  if (!std::getline(in, scoreLine)) return false;
  if (!std::getline(in, nameLine)) return false;

  std::istringstream iss(scoreLine);
  int score = 0;
  iss >> score;
  m_entry.score = score;
  m_entry.name = trim(nameLine);
  if (m_entry.name.empty()) m_entry.name = "---";
  return true;
}

bool HighScoreStore::save() const {
  std::ofstream out(cfg::HighScoreFile, std::ios::trunc);
  if (!out) return false;
  out << m_entry.score << "\n" << m_entry.name << "\n";
  return true;
}

const HighScoreEntry& HighScoreStore::entry() const { return m_entry; }

void HighScoreStore::setIfBetter(int score, const std::string& name) {
  if (score <= m_entry.score) return;
  m_entry.score = score;
  m_entry.name = name.empty() ? "---" : name;
}

