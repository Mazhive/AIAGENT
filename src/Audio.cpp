#include "Audio.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>
#include <vector>

namespace {
constexpr int SampleRate = 44100;

float clamp1(float x) { return std::clamp(x, -1.0f, 1.0f); }

float envAdsr(float t, float dur, float a, float d, float s, float r) {
  if (dur <= 0.0f) return 0.0f;
  const float tRelStart = std::max(0.0f, dur - r);

  if (t < 0.0f) return 0.0f;
  if (t < a) return (a <= 0.0f) ? 1.0f : (t / a);
  if (t < a + d) {
    const float u = (d <= 0.0f) ? 1.0f : ((t - a) / d);
    return 1.0f + (s - 1.0f) * u;
  }
  if (t < tRelStart) return s;
  if (t < dur) {
    const float u = (r <= 0.0f) ? 1.0f : ((t - tRelStart) / r);
    return s * (1.0f - u);
  }
  return 0.0f;
}

float waveSquare(float phase) { return (phase < 0.5f) ? 1.0f : -1.0f; }

float waveTriangle(float phase) {
  const float x = phase < 0.5f ? phase * 2.0f : (1.0f - phase) * 2.0f;
  return (x * 2.0f - 1.0f);
}
} // namespace

bool Audio::init() {
  // Walk: short per-step click with a bit of "SID" grit.
  m_walkBuf = makeTone(Wave::Square, 180.0f, 0.045f, 0.002f, 0.010f, 0.10f, 0.020f, 0.0f, 0.0f, 5);

  // Direction key blips: different pitches for each direction.
  m_upBuf = makeTone(Wave::Square, 740.0f, 0.070f, 0.001f, 0.020f, 0.25f, 0.020f, 6.0f, 0.02f, 4);
  m_downBuf = makeTone(Wave::Square, 330.0f, 0.080f, 0.001f, 0.018f, 0.22f, 0.028f, 6.5f, 0.02f, 4);
  m_leftBuf = makeTone(Wave::Triangle, 520.0f, 0.070f, 0.001f, 0.020f, 0.23f, 0.020f, 7.0f, 0.02f, 4);
  m_rightBuf = makeTone(Wave::Triangle, 610.0f, 0.070f, 0.001f, 0.020f, 0.23f, 0.020f, 7.0f, 0.02f, 4);

  // Explosion: noise burst + falling tone.
  m_explodeBuf = makeExplosion(0.35f);

  m_walk.setBuffer(m_walkBuf);
  m_explode.setBuffer(m_explodeBuf);
  m_up.setBuffer(m_upBuf);
  m_down.setBuffer(m_downBuf);
  m_left.setBuffer(m_leftBuf);
  m_right.setBuffer(m_rightBuf);

  setMasterVolume(m_volume);
  return true;
}

void Audio::setMasterVolume(float vol01) {
  m_volume = std::clamp(vol01, 0.0f, 1.0f);
  const float v = m_volume * 100.0f;
  m_walk.setVolume(v * 0.55f);
  m_explode.setVolume(v);
  m_up.setVolume(v * 0.55f);
  m_down.setVolume(v * 0.55f);
  m_left.setVolume(v * 0.55f);
  m_right.setVolume(v * 0.55f);
}

void Audio::playWalk() {
  // Restart quickly; sounds are tiny.
  m_walk.stop();
  m_walk.play();
}

void Audio::playExplosion() {
  m_explode.stop();
  m_explode.play();
}

void Audio::playDirUp() {
  m_up.stop();
  m_up.play();
}
void Audio::playDirDown() {
  m_down.stop();
  m_down.play();
}
void Audio::playDirLeft() {
  m_left.stop();
  m_left.play();
}
void Audio::playDirRight() {
  m_right.stop();
  m_right.play();
}

sf::SoundBuffer Audio::makeTone(Wave wave, float freqHz, float sec, float attackSec, float decaySec,
                                float sustain, float releaseSec, float vibratoHz, float vibratoDepth,
                                int bitcrush) {
  const int total = std::max(1, static_cast<int>(std::lround(sec * SampleRate)));
  std::vector<sf::Int16> samples(static_cast<size_t>(total));

  float phase = 0.0f;
  float held = 0.0f;
  int holdCounter = 0;
  const int crushHold = std::max(0, bitcrush);

  for (int i = 0; i < total; ++i) {
    const float t = static_cast<float>(i) / SampleRate;
    const float env = envAdsr(t, sec, attackSec, decaySec, sustain, releaseSec);

    const float vib = (vibratoHz > 0.0f && vibratoDepth > 0.0f)
                          ? (1.0f + vibratoDepth * std::sin(2.0f * 3.14159265f * vibratoHz * t))
                          : 1.0f;
    const float f = std::max(1.0f, freqHz * vib);
    phase += f / SampleRate;
    phase -= std::floor(phase);

    float x = 0.0f;
    switch (wave) {
      case Wave::Square: x = waveSquare(phase); break;
      case Wave::Triangle: x = waveTriangle(phase); break;
      case Wave::Noise:
        x = 0.0f;
        break;
    }

    float s = clamp1(x * env);

    // Tiny bitcrush/hold to evoke "SID-ish" crunch.
    if (crushHold > 0) {
      if (holdCounter <= 0) {
        held = s;
        holdCounter = crushHold;
      } else {
        --holdCounter;
      }
      s = held;
    }

    samples[static_cast<size_t>(i)] = static_cast<sf::Int16>(s * 32000.0f);
  }

  sf::SoundBuffer buf;
  buf.loadFromSamples(samples.data(), samples.size(), 1, SampleRate);
  return buf;
}

sf::SoundBuffer Audio::makeExplosion(float sec) {
  const int total = std::max(1, static_cast<int>(std::lround(sec * SampleRate)));
  std::vector<sf::Int16> samples(static_cast<size_t>(total));

  std::mt19937 rng(0xC64u);
  std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

  float phase = 0.0f;
  for (int i = 0; i < total; ++i) {
    const float t = static_cast<float>(i) / SampleRate;
    const float env = envAdsr(t, sec, 0.0015f, 0.06f, 0.0f, 0.20f);

    // Falling tone component.
    const float f = 420.0f * std::exp(-4.5f * t) + 60.0f;
    phase += f / SampleRate;
    phase -= std::floor(phase);
    const float tone = waveSquare(phase) * 0.35f;

    // Noise burst.
    const float noise = dist(rng) * 0.9f;

    float s = clamp1((noise + tone) * env);

    // Hard-ish quantization for grit
    s = std::round(s * 24.0f) / 24.0f;

    samples[static_cast<size_t>(i)] = static_cast<sf::Int16>(s * 32000.0f);
  }

  sf::SoundBuffer buf;
  buf.loadFromSamples(samples.data(), samples.size(), 1, SampleRate);
  return buf;
}

