/*
  OpenDeck CAP1188 Light Console

  Receives OpenDeck OSC touch data:
    /opendeck/sensors/cap1188/touch/0  i
    ...
    /opendeck/sensors/cap1188/touch/7  i
*/

import oscP5.*;
import netP5.*;

OscP5 oscP5;

int oscListenPort = 9000;
String touchPathPrefix = "/opendeck/sensors/cap1188/touch/";

int channelCount = 8;
boolean[] touched = new boolean[channelCount];
boolean[] latched = new boolean[channelCount];
float[] intensity = new float[channelCount];
float[] flash = new float[channelCount];

int lastPacketMs = 0;
boolean receivedData = false;
boolean blackout = false;

void setup() {
  size(1200, 760, P2D);
  surface.setTitle("OpenDeck CAP1188 Light Console");
  oscP5 = new OscP5(this, oscListenPort);
  textFont(createFont("SansSerif", 18));
}

void draw() {
  background(4, 5, 8);
  updateChannels();
  drawRoom();
  drawBeams();
  drawConsole();
  drawHud();
}

void updateChannels() {
  for (int i = 0; i < channelCount; i++) {
    float target = (!blackout && latched[i]) ? 1.0 : 0.0;
    intensity[i] += (target - intensity[i]) * 0.12;
    flash[i] *= 0.86;
  }
}

void drawRoom() {
  noStroke();
  for (int i = 0; i < 10; i++) {
    float amount = i / 9.0;
    fill(10 + amount * 12, 12 + amount * 9, 22 + amount * 18);
    rect(0, height * amount, width, height / 9.0 + 2);
  }

  stroke(255, 210, 120, 28);
  strokeWeight(1);
  line(0, height - 210, width, height - 210);
  line(width / 2.0, 160, 0, height - 210);
  line(width / 2.0, 160, width, height - 210);
}

void drawBeams() {
  colorMode(HSB, 360, 100, 100, 100);
  blendMode(ADD);

  float rigY = 155;
  float floorY = height - 210;

  for (int i = 0; i < channelCount; i++) {
    float amount = intensity[i];

    if (amount <= 0.01) {
      continue;
    }

    float x = map(i, 0, channelCount - 1, 120, width - 120);
    float floorX = width / 2.0 + (x - width / 2.0) * 1.65;
    float beamW = 90 + amount * 165;
    float hue = 190 + i * 18;

    noStroke();
    fill(hue, 70, 95, 12 + amount * 30);
    beginShape();
    vertex(x - 18, rigY);
    vertex(x + 18, rigY);
    vertex(floorX + beamW, floorY);
    vertex(floorX - beamW, floorY);
    endShape(CLOSE);

    fill(hue, 80, 100, 45 + amount * 35);
    ellipse(floorX, floorY, beamW * 1.25, 26 + amount * 46);

    fill(hue, 28, 100, 75);
    ellipse(x, rigY, 28 + flash[i] * 30, 28 + flash[i] * 30);
  }

  blendMode(BLEND);
  colorMode(RGB, 255);
}

void drawConsole() {
  float margin = 58;
  float gap = 14;
  float faderW = (width - margin * 2 - gap * (channelCount - 1)) / channelCount;
  float baseY = height - 175;
  float faderH = 116;

  colorMode(HSB, 360, 100, 100, 100);

  for (int i = 0; i < channelCount; i++) {
    float x = margin + i * (faderW + gap);
    float hue = 190 + i * 18;

    noStroke();
    fill(0, 0, 7, 92);
    rect(x, baseY, faderW, faderH, 7);

    stroke(hue, 60, 75, 74);
    strokeWeight(2);
    fill(hue, 55, 18 + intensity[i] * 42, 75);
    rect(x + 6, baseY + 8, faderW - 12, faderH - 16, 5);

    noStroke();
    fill(hue, 76, 98, 48 + intensity[i] * 45);
    float fillH = (faderH - 20) * intensity[i];
    rect(x + 8, baseY + faderH - 10 - fillH, faderW - 16, fillH, 4);

    fill(0, 0, 100, 92);
    textAlign(CENTER, CENTER);
    textSize(17);
    text(str(i + 1), x + faderW / 2.0, baseY + faderH + 22);
  }

  colorMode(RGB, 255);
}

void drawHud() {
  fill(245);
  textAlign(LEFT, TOP);
  textSize(24);
  text("CAP1188 light console", 44, 34);

  textSize(18);
  text("Status: " + statusText(), 44, 74);
  text("OSC: " + touchPathPrefix + "<0..7>", 44, 104);
  text("Keys: c clear, b blackout", 44, 134);
}

String statusText() {
  if (!receivedData) {
    return "waiting for OSC";
  }

  if (millis() - lastPacketMs > 1200) {
    return "stale";
  }

  return blackout ? "blackout" : "receiving OSC";
}

void oscEvent(OscMessage msg) {
  String path = msg.addrPattern();

  if (!path.startsWith(touchPathPrefix) || (msg.arguments().length < 1)) {
    return;
  }

  int index = int(path.substring(touchPathPrefix.length()));

  if ((index < 0) || (index >= channelCount)) {
    return;
  }

  boolean value = msg.get(0).intValue() != 0;

  if (value && !touched[index]) {
    latched[index] = !latched[index];
    flash[index] = 1.0;
  }

  touched[index] = value;
  receivedData = true;
  lastPacketMs = millis();
}

void keyPressed() {
  if ((key == 'c') || (key == 'C')) {
    for (int i = 0; i < channelCount; i++) {
      latched[i] = false;
      flash[i] = 0;
    }
  } else if ((key == 'b') || (key == 'B')) {
    blackout = !blackout;
  }
}
