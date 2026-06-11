/*
  OpenDeck VL53L5CX Touchless Harp

  Requires the Processing Sound library.

  Receives OpenDeck OSC rows:
    /opendeck/sensors/vl53l5cx/row/0  iiiiiiii
    ...
    /opendeck/sensors/vl53l5cx/row/7  iiiiiiii

  Each row carries eight distance values in millimeters. Invalid zones are 0.
*/

import oscP5.*;
import netP5.*;
import processing.sound.*;

OscP5 oscP5;
int voiceCount = 16;
SinOsc[] voices = new SinOsc[voiceCount];
float[] voiceAmp = new float[voiceCount];
float[] voiceOutAmp = new float[voiceCount];
float[] voiceDecay = new float[voiceCount];
float[] voiceFreq = new float[voiceCount];

int oscListenPort = 9000;
String rowPathPrefix = "/opendeck/sensors/vl53l5cx/row/";

int cols = 8;
int rows = 8;
int zoneCount = cols * rows;

int[] depths = new int[zoneCount];
float[] visualDepths = new float[zoneCount];
int[] missingFrames = new int[zoneCount];
boolean[] foreground = new boolean[zoneCount];
boolean[] visited = new boolean[zoneCount];
int[] queue = new int[zoneCount];

int invalidHoldFrames = 12;
float validSmoothing = 0.25;
float invalidFade = 0.94;

int minUsableMm = 50;
int foregroundCutoffMm = 1000;
int foregroundHysteresisMm = 150;
int minBlobZones = 2;
int minSoundBlobZones = 5;
int soundGateMm = 900;

boolean receivedData = false;
boolean muted = false;

int activeZones = 0;
int foregroundZones = 0;
int blobZones = 0;
int blobNearestMm = 0;
float blobX = 0.0;
float blobY = 0.0;
float blobSize = 0.0;
float smoothBlobX = 0.0;
float smoothBlobY = 0.0;
float smoothBlobSize = 0.0;
boolean blobPresent = false;

float pitchHz = 220.0;
float masterOutAmp = 0.0;
float hitAmp = 0.0;
int lastPad = -1;
int lastHitFrame = 0;
int nextStepFrame = 0;
int stepIndex = 0;
int nextVoice = 0;
int heldXNote = 3;
int heldRegister = 1;
int hitCooldownFrames = 10;
int previousNearestMm = 0;
int strikeDeltaMm = 70;
int padCols = 8;
int padRows = 8;
int[] scaleSemitones = { 0, 2, 5, 7, 9, 12, 14, 17 };
int[] rootSemitones = { 0, 0, 2, 2, 5, 5, 7, 7 };
int[] pattern = { 0, 2, 4, 2, 5, 4, 2, 1, 3, 2, 4, 0 };
float xyPitchGain = 1.45;
float yPitchGain = 1.65;
float maxPitchHz = 1100.0;
float noteLaneHysteresis = 0.90;
float masterGain = 0.08;
float maxMixLevel = 0.85;
float voiceAttack = 0.20;
float voiceRelease = 0.025;
float reusableVoiceLevel = 0.012;

boolean usableDistance(float distance) {
  return distance >= minUsableMm;
}

void setup() {
  size(1100, 900, P2D);
  surface.setTitle("OpenDeck VL53L5CX Touchless Harp");
  oscP5 = new OscP5(this, oscListenPort);

  textFont(createFont("SansSerif", 18));

  for (int idx = 0; idx < voiceCount; idx++) {
    voices[idx] = new SinOsc(this);
    voices[idx].play(220.0, 0.0);
    voiceAmp[idx] = 0.0;
    voiceOutAmp[idx] = 0.0;
    voiceDecay[idx] = 0.94;
    voiceFreq[idx] = 220.0;
  }

  for (int idx = 0; idx < zoneCount; idx++) {
    depths[idx] = 0;
    visualDepths[idx] = 0.0;
    missingFrames[idx] = invalidHoldFrames + 1;
    foreground[idx] = false;
    visited[idx] = false;
  }
}

void draw() {
  background(7);
  updateVisualState();
  findLargestBlob();
  updateSound();
  drawInstrument();
  drawStatus();
}

void updateVisualState() {
  activeZones = 0;
  foregroundZones = 0;

  for (int idx = 0; idx < zoneCount; idx++) {
    if (depths[idx] > 0) {
      visualDepths[idx] = lerp(visualDepths[idx], depths[idx], validSmoothing);
    } else if (missingFrames[idx] > invalidHoldFrames) {
      visualDepths[idx] *= invalidFade;
    }

    float distance = visualDepths[idx];

    if (usableDistance(distance)) {
      activeZones++;
    }

    int cutoff = foreground[idx] ? foregroundCutoffMm + foregroundHysteresisMm : foregroundCutoffMm;
    foreground[idx] = usableDistance(distance) && (distance <= cutoff);

    if (foreground[idx]) {
      foregroundZones++;
    }

    visited[idx] = false;
  }
}

void findLargestBlob() {
  int bestCount = 0;
  int bestNearest = 0;
  float bestWeightedX = 0.0;
  float bestWeightedY = 0.0;
  float bestWeight = 0.0;

  for (int idx = 0; idx < zoneCount; idx++) {
    if (!foreground[idx] || visited[idx]) {
      continue;
    }

    int head = 0;
    int tail = 0;
    int count = 0;
    int nearest = 0;
    float weightedX = 0.0;
    float weightedY = 0.0;
    float totalWeight = 0.0;

    queue[tail++] = idx;
    visited[idx] = true;

    while (head < tail) {
      int current = queue[head++];
      int x = current % cols;
      int y = current / cols;
      int distance = int(visualDepths[current]);
      float weight = constrain(map(distance, foregroundCutoffMm, minUsableMm, 0.1, 1.0), 0.1, 1.0);

      count++;
      weightedX += x * weight;
      weightedY += y * weight;
      totalWeight += weight;

      if ((nearest == 0) || (distance < nearest)) {
        nearest = distance;
      }

      for (int yy = max(0, y - 1); yy <= min(rows - 1, y + 1); yy++) {
        for (int xx = max(0, x - 1); xx <= min(cols - 1, x + 1); xx++) {
          int neighbor = xx + yy * cols;

          if (visited[neighbor] || !foreground[neighbor]) {
            continue;
          }

          visited[neighbor] = true;
          queue[tail++] = neighbor;
        }
      }
    }

    if (count > bestCount) {
      bestCount = count;
      bestNearest = nearest;
      bestWeightedX = weightedX;
      bestWeightedY = weightedY;
      bestWeight = totalWeight;
    }
  }

  blobPresent = bestCount >= minBlobZones;
  blobZones = blobPresent ? bestCount : 0;
  blobNearestMm = blobPresent ? bestNearest : 0;

  if (blobPresent && (bestWeight > 0.0)) {
    blobX = bestWeightedX / bestWeight;
    blobY = bestWeightedY / bestWeight;
    blobSize = float(bestCount) / float(zoneCount);

    smoothBlobX = lerp(smoothBlobX, blobX, 0.10);
    smoothBlobY = lerp(smoothBlobY, blobY, 0.10);
    smoothBlobSize = lerp(smoothBlobSize, blobSize, 0.14);
  } else {
    smoothBlobSize = lerp(smoothBlobSize, 0.0, 0.12);
  }
}

void updateSound() {
  float nearNorm = blobNearestMm > 0 ? constrain(map(blobNearestMm, foregroundCutoffMm, minUsableMm, 0.0, 1.0), 0.0, 1.0) : 0.0;
  boolean soundArmed = blobPresent &&
                       (blobZones >= minSoundBlobZones) &&
                       (blobNearestMm > 0) &&
                       (blobNearestMm <= soundGateMm);

  if (soundArmed && !muted) {
    int pad = currentPad();
    boolean movedCloser = (previousNearestMm > 0) &&
                          (blobNearestMm > 0) &&
                          ((previousNearestMm - blobNearestMm) >= strikeDeltaMm);
    int stepInterval = int(lerp(28, 8, nearNorm));

    if (frameCount >= nextStepFrame) {
      playGestureNote(nearNorm, false);
      nextStepFrame = frameCount + stepInterval;
    }

    if (((pad != lastPad) || movedCloser) && ((frameCount - lastHitFrame) >= hitCooldownFrames)) {
      playGestureNote(nearNorm, true);
      hitAmp = constrain(0.18 + nearNorm * 0.42 + smoothBlobSize * 0.45, 0.0, 0.78);
      lastPad = pad;
      lastHitFrame = frameCount;
    }
  }

  if (soundArmed && (blobNearestMm > 0)) {
    previousNearestMm = blobNearestMm;
  } else {
    previousNearestMm = 0;
    lastPad = -1;
    nextStepFrame = frameCount + 8;
  }

  float output = 0.0;

  for (int idx = 0; idx < voiceCount; idx++) {
    voiceAmp[idx] *= voiceDecay[idx];
    float glide = voiceAmp[idx] > voiceOutAmp[idx] ? voiceAttack : voiceRelease;
    voiceOutAmp[idx] = lerp(voiceOutAmp[idx], voiceAmp[idx], glide);

    if ((voiceAmp[idx] < 0.0005) && (voiceOutAmp[idx] < 0.0005)) {
      voiceAmp[idx] = 0.0;
      voiceOutAmp[idx] = 0.0;
    }

    output = max(output, voiceOutAmp[idx]);

    voices[idx].freq(voiceFreq[idx]);
    voices[idx].amp(muted ? 0.0 : voiceOutAmp[idx] * masterGain);
  }

  masterOutAmp = lerp(masterOutAmp, muted ? 0.0 : output, 0.28);
}

int currentPad() {
  int x = constrain(round(expandedBlobX()), 0, padCols - 1);
  int y = constrain(round(expandedBlobY()), 0, padRows - 1);

  return x + y * padCols;
}

float expandedBlobX() {
  float centered = (smoothBlobX / float(cols - 1)) - 0.5;

  return constrain((centered * xyPitchGain + 0.5) * float(padCols - 1), 0.0, float(padCols - 1));
}

float expandedBlobY() {
  float centered = (smoothBlobY / float(rows - 1)) - 0.5;

  return constrain((centered * yPitchGain + 0.5) * float(padRows - 1), 0.0, float(padRows - 1));
}

void playGestureNote(float nearNorm, boolean accent) {
  float x = expandedBlobX();
  float y = expandedBlobY();
  int wantedXNote = constrain(round(x), 0, padCols - 1);
  float yMusic = map(y, padRows - 1, 0, 0, 4);
  int wantedRegister = constrain(floor(yMusic), 0, 4);

  if (abs(x - heldXNote) > noteLaneHysteresis) {
    heldXNote = wantedXNote;
  }

  if (abs(yMusic - heldRegister) > 0.55) {
    heldRegister = wantedRegister;
  }

  int rootIndex = constrain(round((float(heldXNote) / float(padCols - 1)) * float(rootSemitones.length - 1)), 0, rootSemitones.length - 1);
  int register = heldRegister;
  int patternIndex = pattern[stepIndex % pattern.length];
  int yNoteOffset = constrain(round(yMusic * 0.5), 0, 2);
  int xNoteOffset = constrain(round(map(heldXNote, 0, padCols - 1, 2, 0)), 0, 2);
  int noteIndex = constrain((patternIndex + xNoteOffset + yNoteOffset) % scaleSemitones.length, 0, scaleSemitones.length - 1);
  int semitone = rootSemitones[rootIndex] + scaleSemitones[noteIndex] + register * 5;
  float frequency = min(146.83 * pow(2.0, semitone / 12.0), maxPitchHz);
  float amplitude = constrain((accent ? 0.14 : 0.055) + nearNorm * 0.08 + smoothBlobSize * 0.10, 0.0, 0.24);

  triggerVoice(frequency, amplitude, accent ? 0.93 : 0.965);

  pitchHz = frequency;
  stepIndex++;
}

void triggerVoice(float frequency, float amplitude, float decay) {
  if (currentMixLevel() >= maxMixLevel) {
    return;
  }

  int voice = quietestVoice();

  if (voiceOutAmp[voice] > reusableVoiceLevel) {
    return;
  }

  voiceFreq[voice] = frequency;
  voiceAmp[voice] = amplitude;
  voiceDecay[voice] = decay;
}

float currentMixLevel() {
  float level = 0.0;

  for (int idx = 0; idx < voiceCount; idx++) {
    level += voiceOutAmp[idx];
  }

  return level;
}

int quietestVoice() {
  int voice = nextVoice;
  float quietest = voiceOutAmp[voice];

  for (int idx = 0; idx < voiceCount; idx++) {
    if (voiceOutAmp[idx] < quietest) {
      voice = idx;
      quietest = voiceOutAmp[idx];
    }
  }

  nextVoice = (voice + 1) % voiceCount;

  return voice;
}

void drawInstrument() {
  float panelX = 90;
  float panelY = 95;
  float panelW = width - 180;
  float panelH = height - 290;

  noStroke();
  fill(14);
  rect(panelX, panelY, panelW, panelH, 24);

  for (int y = 0; y < rows; y++) {
    for (int x = 0; x < cols; x++) {
      int idx = x + y * cols;
      float cellW = panelW / cols;
      float cellH = panelH / rows;
      float distance = visualDepths[idx];

      if (!usableDistance(distance)) {
        fill(24);
      } else if (foreground[idx]) {
        float heat = constrain(map(distance, foregroundCutoffMm, minUsableMm, 0.0, 1.0), 0.0, 1.0);
        colorMode(HSB, 360, 100, 100, 100);
        fill(310 - heat * 270, 85, 40 + heat * 60, 92);
        colorMode(RGB, 255);
      } else {
        fill(18, 38, 56);
      }

      rect(panelX + x * cellW + 4, panelY + y * cellH + 4, cellW - 8, cellH - 8, 18);
    }
  }

  if (smoothBlobSize > 0.01) {
    float x = panelX + (expandedBlobX() + 0.5) * (panelW / padCols);
    float y = panelY + (expandedBlobY() + 0.5) * (panelH / padRows);
    float diameter = max(34, smoothBlobSize * panelW * 0.55);

    noFill();
    stroke(255);
    strokeWeight(5);
    ellipse(x, y, diameter, diameter);

    fill(255);
    noStroke();
    ellipse(x, y, 14, 14);
  }

  drawPads(panelX, panelY, panelW, panelH);
  drawMeter(panelX, panelY + panelH + 32, panelW, 24, masterOutAmp / 0.78, color(255, 180, 20));
  drawMeter(panelX, panelY + panelH + 72, panelW, 24, constrain((pitchHz - 90.0) / (1400.0 - 90.0), 0.0, 1.0), color(80, 210, 255));
}

void drawPads(float panelX, float panelY, float panelW, float panelH) {
  float padW = panelW / padCols;
  float padH = panelH / padRows;
  int activePad = blobPresent ? currentPad() : -1;

  noFill();
  strokeWeight(3);

  for (int y = 0; y < padRows; y++) {
    for (int x = 0; x < padCols; x++) {
      int pad = x + y * padCols;

      stroke(pad == activePad ? color(255) : color(255, 80));
      rect(panelX + x * padW + 12, panelY + y * padH + 12, padW - 24, padH - 24, 18);
    }
  }
}

void drawMeter(float x, float y, float w, float h, float value, color c) {
  noStroke();
  fill(28);
  rect(x, y, w, h, 12);
  fill(c);
  rect(x, y, w * constrain(value, 0.0, 1.0), h, 12);
}

void drawStatus() {
  fill(235);
  textAlign(LEFT, TOP);
  textSize(18);

  String dataText = receivedData ? "receiving OSC" : "waiting for OSC on port " + oscListenPort;
  String blobText = blobPresent ? "present" : "none";
  float xNorm = blobPresent ? blobX / float(cols - 1) : 0.0;
  float yNorm = blobPresent ? blobY / float(rows - 1) : 0.0;
  int padX = lastPad >= 0 ? lastPad % padCols : -1;
  int padY = lastPad >= 0 ? lastPad / padCols : -1;

  text("VL53L5CX touchless harp", 90, 34);
  text("Status: " + dataText + (muted ? "   MUTED" : ""), 90, 62);
  text("Blob: " + blobText + "   x=" + nf(xNorm, 1, 2) + " y=" + nf(yNorm, 1, 2) + " size=" + nf(blobSize, 1, 2), 90, height - 92);
  text("Pad: " + (lastPad >= 0 ? padX + "," + padY : "-") + "   Pitch: " + int(pitchHz) + " Hz   Energy: " + nf(hitAmp, 1, 2) + "   Nearest: " + (blobNearestMm > 0 ? blobNearestMm + " mm" : "-"), 90, height - 62);
  text("Keys: m mute, [ ] cutoff (" + foregroundCutoffMm + " mm), - + min blob zones (" + minBlobZones + "), sound zones >= " + minSoundBlobZones, 90, height - 32);
}

void oscEvent(OscMessage message) {
  String address = message.addrPattern();

  if (!address.startsWith(rowPathPrefix) || !message.checkTypetag("iiiiiiii")) {
    return;
  }

  int row = int(address.substring(rowPathPrefix.length()));

  if ((row < 0) || (row >= rows)) {
    return;
  }

  for (int x = 0; x < cols; x++) {
    int idx = (cols - 1 - x) + (rows - 1 - row) * cols;
    int distance = message.get(x).intValue();

    depths[idx] = distance;

    if (distance > 0) {
      missingFrames[idx] = 0;
    } else {
      missingFrames[idx]++;
    }
  }

  receivedData = true;
}

void keyPressed() {
  if ((key == 'm') || (key == 'M')) {
    muted = !muted;
  } else if (key == '[') {
    foregroundCutoffMm = max(minUsableMm, foregroundCutoffMm - 100);
  } else if (key == ']') {
    foregroundCutoffMm += 100;
  } else if (key == '-') {
    minBlobZones = max(1, minBlobZones - 1);
  } else if ((key == '=') || (key == '+')) {
    minBlobZones = min(zoneCount, minBlobZones + 1);
  }
}
