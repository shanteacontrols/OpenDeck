/*
  OpenDeck VL53L4CX Touchless Resonator

  Requires the Processing Sound library.

  Receives OpenDeck OSC distance data:
    /opendeck/sensors/vl53l4cx/distance_norm  f
*/

import oscP5.*;
import netP5.*;
import processing.sound.*;

OscP5 oscP5;
TriOsc stringBody;
SinOsc tone;
SinOsc shimmer;
SinOsc stringHarmonic;
SinOsc upperHarmonic;
SinOsc fourthHarmonic;
SinOsc fifthHarmonic;
LowPass bodyFilter;
Reverb bodyReverb;
int voiceCount = 8;
SinOsc[] voices = new SinOsc[voiceCount];
float[] voiceAmp = new float[voiceCount];
float[] voiceOutAmp = new float[voiceCount];
float[] voiceDecay = new float[voiceCount];
float[] voiceFreq = new float[voiceCount];

int oscListenPort = 9000;
String normPath = "/opendeck/sensors/vl53l4cx/distance_norm";

float firmwareNormalizedDistance = 0.0;
float visualNorm = 0.0;
float fastNorm = 0.0;
float slowNorm = 0.0;
float pitchHz = 220.0;
float targetPitchHz = 220.0;
float outputAmp = 0.0;
float targetAmp = 0.0;
float harmonicAmp = 0.0;
float pulse = 0.0;
float previousNorm = 0.0;

int lastPacketMs = 0;
int lastStrikeFrame = 0;
int nextAutoPluckFrame = 0;
int nextVoice = 0;
boolean receivedData = false;
boolean muted = false;

float minPitchHz = 293.66;
float maxPitchHz = 587.33;
float masterGain = 0.16;
float wobbleSensitivity = 72.0;
float pitchCenterPull = 0.88;
float vibratoPhase = 0.0;
float[] musicalCenters = {
  293.66, 311.13, 349.23, 392.00, 440.00,
  466.16, 523.25, 587.33
};
float strikeThreshold = 0.018;
int strikeCooldownFrames = 6;

void setup() {
  size(1200, 900, P2D);
  surface.setTitle("OpenDeck VL53L4CX Touchless Resonator");

  oscP5 = new OscP5(this, oscListenPort);
  textFont(createFont("SansSerif", 18));

  stringBody = new TriOsc(this);
  stringBody.play(220.0, 0.0);

  tone = new SinOsc(this);
  tone.play(220.0, 0.0);

  shimmer = new SinOsc(this);
  shimmer.play(440.0, 0.0);

  stringHarmonic = new SinOsc(this);
  stringHarmonic.play(660.0, 0.0);

  upperHarmonic = new SinOsc(this);
  upperHarmonic.play(880.0, 0.0);

  fourthHarmonic = new SinOsc(this);
  fourthHarmonic.play(1174.0, 0.0);

  fifthHarmonic = new SinOsc(this);
  fifthHarmonic.play(1468.0, 0.0);

  bodyFilter = new LowPass(this);
  bodyFilter.process(stringBody, 1250);

  bodyReverb = new Reverb(this);
  bodyReverb.process(stringBody);
  bodyReverb.room(0.72);
  bodyReverb.damp(0.82);
  bodyReverb.wet(0.18);

  for (int idx = 0; idx < voiceCount; idx++) {
    voices[idx] = new SinOsc(this);
    voices[idx].play(220.0, 0.0);
    voiceAmp[idx] = 0.0;
    voiceOutAmp[idx] = 0.0;
    voiceDecay[idx] = 0.965;
    voiceFreq[idx] = 220.0;
  }
}

void draw() {
  fastNorm += (firmwareNormalizedDistance - fastNorm) * 0.88;
  slowNorm += (firmwareNormalizedDistance - slowNorm) * 0.12;
  visualNorm += (firmwareNormalizedDistance - visualNorm) * 0.70;

  float closeness = closenessAmount();

  updateSound(closeness);
  updateStrikeVoices();

  background(5 + closeness * 20, 8 + closeness * 20, 14 + closeness * 22);

  pulse += 0.02 + outputAmp * 0.18;
  vibratoPhase += 0.54 + outputAmp * 0.045;

  drawField(closeness);
  drawInstrument(closeness);
  drawMeter(closeness);
  drawHud();
}

float closenessAmount() {
  return constrain(1.0 - visualNorm, 0.0, 1.0);
}

void updateSound(float closeness) {
  float baseCloseness = constrain(1.0 - slowNorm, 0.0, 1.0);
  float playable = constrain(map(baseCloseness, 0.02, 0.96, 0.0, 1.0), 0.0, 1.0);
  float expressive = pow(playable, 0.95);
  float basePitch = minPitchHz * pow(maxPitchHz / minPitchHz, expressive);
  float centeredPitch = pitchWithMusicalCenter(basePitch);

  targetPitchHz = centeredPitch;

  targetAmp = receivedData && !stale() ? constrain(map(closeness, 0.03, 0.70, 0.0, 0.85), 0.0, 0.85) : 0.0;

  pitchHz += (targetPitchHz - pitchHz) * 0.18;
  outputAmp += (targetAmp - outputAmp) * 0.075;
  harmonicAmp += (targetAmp - harmonicAmp) * 0.06;

  float vibratoSemitones = sin(vibratoPhase) * (0.05 + outputAmp * 0.16);
  float handVibratoSemitones = constrain((slowNorm - fastNorm) * wobbleSensitivity, -1.7, 1.7);
  float voicedPitch = constrain(pitchHz * pow(2.0, (vibratoSemitones + handVibratoSemitones) / 12.0), minPitchHz, maxPitchHz);
  float bowMotion = 0.97 + sin(vibratoPhase * 0.73) * 0.03;

  bodyFilter.freq(950 + outputAmp * 850);

  stringBody.freq(voicedPitch);
  stringBody.amp(0.0);

  tone.freq(voicedPitch);
  tone.amp(0.0);

  shimmer.freq(voicedPitch * 1.006);
  shimmer.amp(0.0);

  stringHarmonic.freq(voicedPitch * 2.0);
  stringHarmonic.amp(0.0);

  upperHarmonic.freq(voicedPitch * 3.0);
  upperHarmonic.amp(0.0);

  fourthHarmonic.freq(voicedPitch * 4.0);
  fourthHarmonic.amp(muted ? 0.0 : harmonicAmp * masterGain * 0.0);

  fifthHarmonic.freq(voicedPitch * 5.0);
  fifthHarmonic.amp(muted ? 0.0 : harmonicAmp * masterGain * 0.0);
}

void updateStrikeVoices() {
  if (!receivedData || stale() || muted) {
    fadeStrikeVoices();
    return;
  }

  float delta = abs(firmwareNormalizedDistance - previousNorm);
  float closeness = constrain(1.0 - firmwareNormalizedDistance, 0.0, 1.0);

  if ((delta >= strikeThreshold) && ((frameCount - lastStrikeFrame) >= strikeCooldownFrames)) {
    triggerStrike(delta, true);
    lastStrikeFrame = frameCount;
  }

  if ((closeness > 0.04) && (frameCount >= nextAutoPluckFrame)) {
    triggerStrike(0.006 + closeness * 0.010, false);
    nextAutoPluckFrame = frameCount + int(lerp(42, 16, closeness));
  }

  previousNorm = firmwareNormalizedDistance;
  fadeStrikeVoices();
}

void triggerStrike(float delta, boolean accent) {
  float closeness = constrain(1.0 - firmwareNormalizedDistance, 0.0, 1.0);
  int noteIndex = constrain(round(closeness * float(musicalCenters.length - 1)), 0, musicalCenters.length - 1);
  int voice = nextVoice;

  nextVoice = (nextVoice + 1) % voiceCount;
  voiceFreq[voice] = musicalCenters[noteIndex];
  voiceAmp[voice] = accent
                  ? constrain(0.16 + delta * 7.5, 0.0, 0.50)
                  : constrain(0.055 + closeness * 0.07, 0.0, 0.16);
  voiceDecay[voice] = accent
                    ? 0.955 + constrain(closeness * 0.025, 0.0, 0.025)
                    : 0.972 + constrain(closeness * 0.014, 0.0, 0.014);
}

void fadeStrikeVoices() {
  for (int idx = 0; idx < voiceCount; idx++) {
    voiceAmp[idx] *= voiceDecay[idx];
    voiceOutAmp[idx] = lerp(voiceOutAmp[idx], voiceAmp[idx], voiceAmp[idx] > voiceOutAmp[idx] ? 0.32 : 0.035);

    if ((voiceAmp[idx] < 0.0005) && (voiceOutAmp[idx] < 0.0005)) {
      voiceAmp[idx] = 0.0;
      voiceOutAmp[idx] = 0.0;
    }

    voices[idx].freq(voiceFreq[idx]);
    voices[idx].amp(muted ? 0.0 : voiceOutAmp[idx] * masterGain * 0.92);
  }
}

float pitchWithMusicalCenter(float pitch) {
  float nearest = musicalCenters[0];
  float bestDistance = abs(log(pitch / nearest));

  for (int i = 1; i < musicalCenters.length; i++) {
    float distance = abs(log(pitch / musicalCenters[i]));

    if (distance < bestDistance) {
      bestDistance = distance;
      nearest = musicalCenters[i];
    }
  }

  return exp(lerp(log(pitch), log(nearest), pitchCenterPull));
}

void drawField(float closeness) {
  noStroke();

  for (int i = 10; i >= 1; i--) {
    float amount = i / 10.0;
    float w = width * (0.12 + amount * 0.85);
    float h = height * (0.06 + amount * 0.52);

    fill(25 + closeness * 95, 105 + closeness * 120, 255 - closeness * 125, (5 + closeness * 18) * amount);
    ellipse(width / 2.0, height / 2.0 + 5, w, h);
  }

  for (int i = 0; i < 42; i++) {
    float x = map(i, 0, 41, 120, width - 120);
    float offset = sin(pulse + i * 0.38) * (10 + closeness * 40);
    float alpha = 28 + closeness * 80;

    stroke(90 + closeness * 145, 180 + closeness * 65, 255 - closeness * 120, alpha);
    strokeWeight(1.0 + closeness * 2.5);
    line(x, 260 + offset, x, height - 190 - offset * 0.35);
  }
}

void drawInstrument(float closeness) {
  float centerX = width / 2.0;
  float centerY = height / 2.0 + 20;
  float radius = 86 + closeness * 250;
  float ringRadius = radius + sin(pulse * 2.1) * (10 + closeness * 32);

  noStroke();
  fill(255, 228 - closeness * 90, 80 + closeness * 60, 200 + outputAmp * 55);
  ellipse(centerX, centerY, radius, radius);

  fill(255, 255, 230, 155);
  ellipse(centerX - radius * 0.15, centerY - radius * 0.18, radius * 0.22, radius * 0.22);

  noFill();
  stroke(255, 238, 120, 90 + outputAmp * 165);
  strokeWeight(3 + outputAmp * 10);
  ellipse(centerX, centerY, ringRadius, ringRadius);

  float pitchY = map(pitchHz, minPitchHz, maxPitchHz, centerY + 250, centerY - 250);

  stroke(255, 255, 255, 80);
  strokeWeight(1.5);
  line(centerX - 330, pitchY, centerX + 330, pitchY);

  fill(245);
  textAlign(CENTER, CENTER);
  textSize(28);
  text(nf(pitchHz, 1, 1) + " Hz", centerX, centerY + radius * 0.70);
}

void drawMeter(float closeness) {
  float w = min(width - 280, 720);
  float x = (width - w) / 2.0;
  float y = height - 92;
  float h = 28;

  rectMode(CORNER);

  noStroke();
  fill(22, 24, 30);
  rect(x, y, w, h, 7);

  fill(55 + closeness * 200, 175 + closeness * 70, 255 - closeness * 160);
  rect(x, y, w * closeness, h, 7);

  noFill();
  stroke(245, 180);
  strokeWeight(2);
  rect(x, y, w, h, 7);

  fill(245);
  textSize(18);
  textAlign(CENTER, CENTER);
  text("normalized " + nf(firmwareNormalizedDistance, 1, 3), x + w / 2.0, y + h / 2.0);
}

void drawHud() {
  fill(245);
  textAlign(LEFT, TOP);
  textSize(24);
  text("VL53L4CX touchless resonator", 42, 38);

  textSize(18);
  text("Status: " + statusText(), 42, 78);
  text("Input: firmware normalized", 42, 108);
  text("Sound: " + (muted ? "muted" : "on") + " / continuous", 42, 138);
  text("Keys: space mute", 42, 168);
  text("OSC: " + normPath, 42, 198);
}

String statusText() {
  if (!receivedData) {
    return "waiting for OSC";
  }

  if (stale()) {
    return "stale";
  }

  return "receiving OSC";
}

boolean stale() {
  return millis() - lastPacketMs > 1200;
}

void keyPressed() {
  if (key == ' ') {
    muted = !muted;
  }
}

void oscEvent(OscMessage message) {
  if (message.checkAddrPattern(normPath)) {
    if (message.typetag().length() < 1) {
      return;
    }

    if (message.typetag().charAt(0) == 'f') {
      firmwareNormalizedDistance = constrain(message.get(0).floatValue(), 0.0, 1.0);
    } else if (message.typetag().charAt(0) == 'i') {
      firmwareNormalizedDistance = constrain(message.get(0).intValue(), 0.0, 1.0);
    } else {
      return;
    }

    if (!receivedData) {
      fastNorm = firmwareNormalizedDistance;
      slowNorm = firmwareNormalizedDistance;
      visualNorm = firmwareNormalizedDistance;
      previousNorm = firmwareNormalizedDistance;
    }

    markPacket();
  }
}

void markPacket() {
  receivedData = true;
  lastPacketMs = millis();
}
