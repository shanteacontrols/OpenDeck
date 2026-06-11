/*
  OpenDeck CAP1188 Touch Piano

  Requires the Processing Sound library.

  Receives OpenDeck OSC touch data:
    /opendeck/sensors/cap1188/touch/0  i
    ...
    /opendeck/sensors/cap1188/touch/7  i
*/

import oscP5.*;
import netP5.*;
import processing.sound.*;

OscP5 oscP5;

int oscListenPort = 9000;
String touchPathPrefix = "/opendeck/sensors/cap1188/touch/";

int padCount = 8;
boolean[] touched = new boolean[padCount];
float[] padGlow = new float[padCount];
float[] padPulse = new float[padCount];

int lastPacketMs = 0;
boolean receivedData = false;
boolean muted = false;

int voiceCount = 16;
SinOsc[] voices = new SinOsc[voiceCount];
float[] voiceAmp = new float[voiceCount];
float[] voiceOutAmp = new float[voiceCount];
float[] voiceFreq = new float[voiceCount];
float[] voiceDecay = new float[voiceCount];
int nextVoice = 0;

float[] noteHz = {
  261.63, 293.66, 329.63, 392.00,
  440.00, 523.25, 587.33, 659.25
};

void setup() {
  size(1200, 760, P2D);
  surface.setTitle("OpenDeck CAP1188 Touch Piano");
  oscP5 = new OscP5(this, oscListenPort);
  textFont(createFont("SansSerif", 18));

  for (int i = 0; i < voiceCount; i++) {
    voices[i] = new SinOsc(this);
    voices[i].play(220, 0);
    voiceAmp[i] = 0;
    voiceOutAmp[i] = 0;
    voiceFreq[i] = 220;
    voiceDecay[i] = 0.94;
  }
}

void draw() {
  background(7, 8, 12);
  updateVoices();
  drawStage();
  drawPads();
  drawHud();
}

void updateVoices() {
  for (int i = 0; i < voiceCount; i++) {
    voiceAmp[i] *= voiceDecay[i];
    if (voiceAmp[i] < 0.0005) {
      voiceAmp[i] = 0;
    }

    voiceOutAmp[i] += (voiceAmp[i] - voiceOutAmp[i]) * 0.24;
    voices[i].freq(voiceFreq[i]);
    voices[i].amp(muted ? 0 : voiceOutAmp[i]);
  }
}

void drawStage() {
  noStroke();
  for (int i = 0; i < 8; i++) {
    float amount = i / 7.0;
    fill(18 + amount * 16, 20 + amount * 12, 34 + amount * 18, 120);
    rect(0, height * amount, width, height / 7.0 + 2);
  }

  stroke(255, 210, 110, 45);
  strokeWeight(1);
  for (int i = 0; i < 26; i++) {
    float x = map(i, 0, 25, 70, width - 70);
    line(x, height - 210, width / 2.0 + (x - width / 2.0) * 0.42, 210);
  }
}

void drawPads() {
  float margin = 70;
  float gap = 12;
  float padW = (width - margin * 2 - gap * (padCount - 1)) / padCount;
  float padH = 310;
  float y = height - padH - 90;

  colorMode(HSB, 360, 100, 100, 100);

  for (int i = 0; i < padCount; i++) {
    float x = margin + i * (padW + gap);
    padGlow[i] += ((touched[i] ? 1.0 : 0.0) - padGlow[i]) * 0.18;
    padPulse[i] *= 0.88;

    float hue = 205 + i * 15;
    float lift = padGlow[i] * 26 + padPulse[i] * 18;
    float round = 7;

    noStroke();
    fill(hue, 55, 20 + lift * 0.35, 55);
    rect(x, y + 14, padW, padH, round);

    stroke(hue, 78, 84 + lift, 85);
    strokeWeight(2 + padGlow[i] * 4);
    fill(hue, 64, 30 + lift, 72 + padGlow[i] * 24);
    rect(x, y - lift * 0.35, padW, padH, round);

    noStroke();
    fill(hue - 16, 22, 100, 30 + padGlow[i] * 45);
    rect(x + padW * 0.14, y + 26 - lift * 0.35, padW * 0.48, padH * 0.16, 6);

    fill(0, 0, 100, 92);
    textAlign(CENTER, CENTER);
    textSize(22);
    text(noteName(i), x + padW / 2.0, y + padH - 46);
  }

  colorMode(RGB, 255);
}

void drawHud() {
  fill(245);
  textAlign(LEFT, TOP);
  textSize(24);
  text("CAP1188 touch piano", 44, 38);

  textSize(18);
  text("Status: " + statusText(), 44, 78);
  text("OSC: " + touchPathPrefix + "<0..7>", 44, 108);
  text("Keys: space mute", 44, 138);
}

String statusText() {
  if (!receivedData) {
    return "waiting for OSC";
  }

  if (millis() - lastPacketMs > 1200) {
    return "stale";
  }

  return muted ? "muted" : "receiving OSC";
}

String noteName(int index) {
  String[] names = { "C", "D", "E", "G", "A", "C", "D", "E" };
  return names[index];
}

void triggerNote(int index) {
  int voice = nextVoice;
  nextVoice = (nextVoice + 1) % voiceCount;

  voiceFreq[voice] = noteHz[index];
  voiceAmp[voice] = 0.18;
  voiceOutAmp[voice] = 0.0;
  voiceDecay[voice] = 0.915;
  padPulse[index] = 1.0;
}

void oscEvent(OscMessage msg) {
  String path = msg.addrPattern();

  if (!path.startsWith(touchPathPrefix) || (msg.arguments().length < 1)) {
    return;
  }

  int index = int(path.substring(touchPathPrefix.length()));

  if ((index < 0) || (index >= padCount)) {
    return;
  }

  boolean value = msg.get(0).intValue() != 0;

  if (value && !touched[index]) {
    triggerNote(index);
  }

  touched[index] = value;
  receivedData = true;
  lastPacketMs = millis();
}

void keyPressed() {
  if (key == ' ') {
    muted = !muted;
  }
}
