#pragma once

#include <SFML/Audio.hpp>

class Audio {
public:
  bool init();

  void playWalk();
  void playExplosion();

  void playDirUp();
  void playDirDown();
  void playDirLeft();
  void playDirRight();

  void setMasterVolume(float vol01);

private:
  enum class Wave { Square, Triangle, Noise };

  static sf::SoundBuffer makeTone(Wave wave, float freqHz, float sec, float attackSec, float decaySec,
                                  float sustain, float releaseSec, float vibratoHz = 0.0f,
                                  float vibratoDepth = 0.0f, int bitcrush = 0);

  static sf::SoundBuffer makeExplosion(float sec);

private:
  float m_volume{0.35f};

  sf::SoundBuffer m_walkBuf;
  sf::SoundBuffer m_explodeBuf;
  sf::SoundBuffer m_upBuf;
  sf::SoundBuffer m_downBuf;
  sf::SoundBuffer m_leftBuf;
  sf::SoundBuffer m_rightBuf;

  sf::Sound m_walk;
  sf::Sound m_explode;
  sf::Sound m_up;
  sf::Sound m_down;
  sf::Sound m_left;
  sf::Sound m_right;
};

