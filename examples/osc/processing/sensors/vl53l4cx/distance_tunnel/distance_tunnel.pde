/*
  OpenDeck VL53L4CX Distance Tunnel

  Receives OpenDeck OSC distance data:
    /opendeck/sensors/vl53l4cx/distance       i
    /opendeck/sensors/vl53l4cx/distance_norm  f
*/

import oscP5.*;
import netP5.*;

OscP5 oscP5;

int oscListenPort = 9000;
String distancePath = "/opendeck/sensors/vl53l4cx/distance";
String normPath = "/opendeck/sensors/vl53l4cx/distance_norm";

int distanceMm = 0;
float rawNormalizedDistance = 0.0;
float firmwareNormalizedDistance = 0.0;
float visualDistance = 0.0;
float visualNorm = 0.0;
float pulse = 0.0;
float shockPulse = 0.0;
float previousCloseness = 0.0;

int packetCount = 0;
int lastPacketMs = 0;
boolean receivedData = false;

int nearMm = 40;
int farMm = 1200;
boolean useFirmwareNormalized = true;

void setup() {
  size(1200, 900, P2D);
  surface.setTitle("OpenDeck VL53L4CX Distance Tunnel");

  oscP5 = new OscP5(this, oscListenPort);
  textFont(createFont("SansSerif", 18));
}

void draw() {
  float targetDistance = distanceMm > 0 ? distanceMm : farMm;
  float targetNorm = activeNormalizedDistance();

  visualDistance += (targetDistance - visualDistance) * 0.16;
  visualNorm += (targetNorm - visualNorm) * 0.16;

  float closeness = closenessDrive();
  float closenessDelta = max(0.0, closeness - previousCloseness);
  previousCloseness = closeness;

  background(4 + closeness * 22, 6 + closeness * 18, 10 + closeness * 26);

  if (closenessDelta > 0.035) {
    shockPulse = 1.0;
  }

  shockPulse *= 0.90;
  pulse += 0.026 + closeness * 0.072;

  drawSceneGlow(closeness);
  drawTunnel();
  drawStreaks();
  drawRangeBeam();
  drawTarget();
  drawMeter();
  drawHud();
}

void drawSceneGlow(float closeness) {
  if (closeness <= 0.01) {
    return;
  }

  float nearPunch = smoothAmount(closeness, 0.48, 1.0);

  noStroke();

  for (int i = 9; i >= 1; i--) {
    float amount = i / 9.0;
    float glowWidth = width * (0.18 + amount * 0.86);
    float glowHeight = height * (0.10 + amount * 0.54);

    fill(30 + nearPunch * 140, 120 + nearPunch * 95, 255 - nearPunch * 95, (5 + nearPunch * 16) * amount);
    ellipse(width / 2.0, height / 2.0 + 20, glowWidth, glowHeight);
  }
}

float closenessDrive() {
  float raw = constrain(1.0 - visualNorm, 0.0, 1.0);
  return constrain(pow(raw, 0.52), 0.0, 1.0);
}

float closeSurgeDrive() {
  if (visualDistance <= 0.0) {
    return 0.0;
  }

  float closeNorm = constrain(map(visualDistance, 440.0, nearMm, 0.0, 1.0), 0.0, 1.0);
  return smoothAmount(closeNorm, 0.0, 1.0);
}

float smoothAmount(float value, float edge0, float edge1) {
  float x = constrain((value - edge0) / (edge1 - edge0), 0.0, 1.0);
  return x * x * (3.0 - 2.0 * x);
}

void drawTunnel() {
  pushMatrix();
  translate(width / 2.0, height / 2.0 + 20);

  float closeness = closenessDrive();
  float closeSurge = closeSurgeDrive();
  float nearPunch = smoothAmount(closeness, 0.62, 1.0);
  int rings = 22;

  noFill();

  for (int i = rings; i >= 1; i--) {
    float amount = i / float(rings);
    float squeeze = 1.0 + nearPunch * pow(1.0 - amount, 1.8) * 1.45 + closeSurge * pow(1.0 - amount, 2.2) * 2.3;
    float wobble = sin(pulse * (1.0 + nearPunch * 1.4 + closeSurge * 2.0) + i * 0.48) * (8.0 + nearPunch * 24.0 + closeSurge * 42.0) * closeness;
    float size = (80 + amount * 760 + wobble) * squeeze;
    float alpha = map(i, rings, 1, 30 + closeness * 50 + closeSurge * 65, 175 + nearPunch * 80 + closeSurge * 60);

    strokeWeight(1.3 + closeness * 3.0 + nearPunch * 6.0 * (1.0 - amount) + closeSurge * 5.5);
    stroke(45 + closeness * 210, 135 + closeness * 95 + closeSurge * 25, 255 - nearPunch * 115 - closeSurge * 60, alpha);

    rectMode(CENTER);
    rect(0, 0, size, size * (0.62 - nearPunch * 0.16 - closeSurge * 0.12), 18 + nearPunch * 26 + closeSurge * 34);
  }

  for (int i = 0; i < 16; i++) {
    float angle = TWO_PI * i / 16.0;
    float x = cos(angle) * 420;
    float y = sin(angle) * 260;

    stroke(70, 135, 190, 70);
    strokeWeight(1);
    line(0, 0, x, y);
  }

  if ((shockPulse > 0.02) || (closeSurge > 0.72)) {
    float shockSize = 220 + (1.0 - max(shockPulse, closeSurge * 0.42)) * 900;
    float shockAlpha = max(180 * shockPulse, 70 * closeSurge);

    stroke(255, 245, 140, shockAlpha);
    strokeWeight(2 + 8 * shockPulse + 4 * closeSurge);
    rectMode(CENTER);
    rect(0, 0, shockSize, shockSize * 0.42, 30);
  }

  popMatrix();
}

void drawStreaks() {
  float closeness = closenessDrive();
  float closeSurge = closeSurgeDrive();
  float nearPunch = smoothAmount(closeness, 0.50, 1.0);

  if (nearPunch <= 0.01) {
    return;
  }

  pushMatrix();
  translate(width / 2.0, height / 2.0 + 20);

  for (int i = 0; i < 46; i++) {
    float angle = TWO_PI * i / 46.0 + sin(pulse * 0.7 + i) * 0.05;
    float start = 90 + ((i * 37 + int(pulse * 80)) % 420);
    float len = 60 + nearPunch * 220 + closeSurge * 280;
    float sx = cos(angle) * start;
    float sy = sin(angle) * start * 0.62;
    float ex = cos(angle) * (start + len);
    float ey = sin(angle) * (start + len) * 0.62;

    stroke(120 + nearPunch * 135, 215 + closeSurge * 30, 255 - nearPunch * 100 - closeSurge * 50, 25 + nearPunch * 120 + closeSurge * 80);
    strokeWeight(1 + nearPunch * 3 + closeSurge * 2);
    line(sx, sy, ex, ey);
  }

  popMatrix();
}

void drawRangeBeam() {
  float closeness = closenessDrive();
  float closeSurge = closeSurgeDrive();
  float nearPunch = smoothAmount(closeness, 0.62, 1.0);
  float centerX = width / 2.0;
  float baseY = height / 2.0 + 20;
  float beamLength = map(visualNorm, 0.0, 1.0, 70, 460) + nearPunch * 120 + closeSurge * 160;

  noStroke();

  for (int i = 9; i >= 1; i--) {
    float amount = i / 9.0;
    fill(50 + nearPunch * 190, 180 + nearPunch * 55 + closeSurge * 35, 255 - nearPunch * 140 - closeSurge * 55, 7 + closeness * 26 + closeSurge * 20);
    ellipse(centerX, baseY, beamLength * amount * (2.0 + nearPunch * 1.2 + closeSurge * 0.8), beamLength * amount * (0.6 + nearPunch * 0.38 + closeSurge * 0.24));
  }

  stroke(120 + nearPunch * 135, 220 + closeSurge * 30, 255 - nearPunch * 150 - closeSurge * 65, 150 + nearPunch * 90 + closeSurge * 70);
  strokeWeight(3 + nearPunch * 6 + closeSurge * 6);
  line(centerX - beamLength, baseY, centerX + beamLength, baseY);
}

void drawTarget() {
  float closeness = closenessDrive();
  float closeSurge = closeSurgeDrive();
  float nearPunch = smoothAmount(closeness, 0.58, 1.0);
  float x = width / 2.0;
  float y = height / 2.0 + 20;
  float radius = 38 + closeness * 150 + nearPunch * 170 + closeSurge * 340;
  float ring = radius + sin(pulse * 2.4) * (8 + closeness * 26 + nearPunch * 45 + closeSurge * 80);

  noStroke();
  fill(255, 230 - closeness * 70, 60 + closeness * 40, 220 + nearPunch * 35);
  ellipse(x, y, radius, radius);

  fill(255, 255, 230, 170);
  ellipse(x - radius * 0.14, y - radius * 0.18, radius * 0.24, radius * 0.24);

  noFill();
  stroke(255, 235, 110, 160 + nearPunch * 80);
  strokeWeight(3 + closeness * 5 + nearPunch * 5);
  ellipse(x, y, ring, ring);

  stroke(255, 235, 110, 90 + nearPunch * 90);
  strokeWeight(1.5 + nearPunch * 3);
  line(x - ring * 0.72, y, x + ring * 0.72, y);
  line(x, y - ring * 0.72, x, y + ring * 0.72);
}

void drawMeter() {
  float w = min(width - 116, 720);
  float x = (width - w) / 2.0;
  float y = height - 96;
  float h = 28;
  float fillAmount = activeMeterAmount();
  float nearPunch = smoothAmount(closenessDrive(), 0.62, 1.0);

  rectMode(CORNER);

  noStroke();
  fill(28);
  rect(x, y, w, h, 7);

  fill(45 + nearPunch * 210, 175 + nearPunch * 55, 255 - nearPunch * 160);
  rect(x, y, w * fillAmount, h, 7);

  noFill();
  stroke(245, 170);
  strokeWeight(2);
  rect(x, y, w, h, 7);

  fill(245);
  textSize(18);

  textAlign(RIGHT, CENTER);
  text(distanceMm + " mm", x - 18, y + h / 2.0);

  textAlign(LEFT, CENTER);
  text("normalized " + nf(activeNormalizedDistance(), 1, 3), x + w + 18, y + h / 2.0);
}

void drawHud() {
  fill(245);
  textAlign(LEFT, TOP);
  textSize(24);
  text("VL53L4CX distance tunnel", 42, 38);

  textSize(18);
  text("Status: " + statusText(), 42, 78);
  text("Packets: " + packetCount, 42, 108);
  text("Mode: " + modeText(), 42, 138);
  text("Keys: m raw mm, n normalized", 42, 168);
  text("OSC: " + distancePath, 42, 198);
  text("OSC: " + normPath, 42, 228);
}

float activeNormalizedDistance() {
  if (useFirmwareNormalized) {
    return firmwareNormalizedDistance;
  }

  return rawNormalizedDistance;
}

float activeMeterAmount() {
  return constrain(1.0 - activeNormalizedDistance(), 0.0, 1.0);
}

String modeText() {
  if (useFirmwareNormalized) {
    return "firmware normalized";
  }

  return "raw millimeters";
}

void keyPressed() {
  if ((key == 'n') || (key == 'N')) {
    useFirmwareNormalized = true;
  } else if ((key == 'm') || (key == 'M')) {
    useFirmwareNormalized = false;
  }
}

String statusText() {
  if (!receivedData) {
    return "waiting for OSC";
  }

  if (millis() - lastPacketMs > 1200) {
    return "stale";
  }

  return "receiving OSC";
}

void oscEvent(OscMessage message) {
  if (message.checkAddrPattern(distancePath)) {
    if (message.typetag().length() < 1) {
      return;
    }

    if (message.typetag().charAt(0) == 'i') {
      distanceMm = max(0, message.get(0).intValue());
    } else if (message.typetag().charAt(0) == 'f') {
      distanceMm = max(0, round(message.get(0).floatValue()));
    } else {
      return;
    }

    if (distanceMm > 0) {
      rawNormalizedDistance = constrain(map(distanceMm, nearMm, farMm, 0.0, 1.0), 0.0, 1.0);
    }

    markPacket();
    return;
  }

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

    markPacket();
  }
}

void markPacket() {
  receivedData = true;
  lastPacketMs = millis();
  packetCount++;
}
