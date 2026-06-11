/*
  OpenDeck APDS9960 Color Studio

  Receives OpenDeck OSC RGB data:
    /opendeck/sensors/apds9960/rgb  fff
*/

import oscP5.*;
import netP5.*;

OscP5 oscP5;

int oscListenPort = 9000;
String rgbPath = "/opendeck/sensors/apds9960/rgb";

float redValue = 0.0;
float greenValue = 0.0;
float blueValue = 0.0;
float visualRed = 0.0;
float visualGreen = 0.0;
float visualBlue = 0.0;
int lastPacketMs = 0;
int packetCount = 0;
boolean receivedData = false;

ArrayList<ColorChip> history = new ArrayList<ColorChip>();

void setup() {
  size(1200, 900, P2D);
  surface.setTitle("OpenDeck APDS9960 Color Studio");

  oscP5 = new OscP5(this, oscListenPort);
  textFont(createFont("SansSerif", 18));
}

void draw() {
  visualRed += (redValue - visualRed) * 0.12;
  visualGreen += (greenValue - visualGreen) * 0.12;
  visualBlue += (blueValue - visualBlue) * 0.12;

  drawBackdrop();
  drawSwatch();
  drawChannels();
  drawHistory();
  drawHud();
}

void drawBackdrop() {
  int sensed = sensedColor(visualRed, visualGreen, visualBlue);
  int dark = lerpColor(color(7, 8, 12), sensed, 0.22);

  background(dark);

  noStroke();
  for (int i = 8; i >= 1; i--) {
    float t = i / 8.0;
    fill(red(sensed), green(sensed), blue(sensed), 10 * t);
    ellipse(width / 2.0, height / 2.0, width * 0.92 * t, height * 0.82 * t);
  }
}

void drawSwatch() {
  float cx = width / 2.0;
  float cy = height / 2.0 - 24;
  float size = min(width, height) * 0.46;
  int sensed = sensedColor(visualRed, visualGreen, visualBlue);

  noStroke();
  fill(0, 90);
  rect(cx - size / 2.0 + 18, cy - size / 2.0 + 22, size, size, 36);

  fill(sensed);
  rect(cx - size / 2.0, cy - size / 2.0, size, size, 36);

  fill(255, 70);
  rect(cx - size / 2.0 + 22, cy - size / 2.0 + 22, size * 0.36, size * 0.18, 20);

  noFill();
  stroke(255, 190);
  strokeWeight(4);
  rect(cx - size / 2.0, cy - size / 2.0, size, size, 36);

  fill(contrastColor(sensed));
  textAlign(CENTER, CENTER);
  textSize(26);
  text(rgbLabel(), cx, cy + size * 0.39);
}

void drawChannels() {
  float x = 72;
  float y = height - 170;
  float w = width - 144;
  float h = 20;
  float gap = 36;

  drawChannel("R", visualRed, color(255, 72, 72), x, y, w, h);
  drawChannel("G", visualGreen, color(70, 235, 110), x, y + gap, w, h);
  drawChannel("B", visualBlue, color(80, 140, 255), x, y + gap * 2, w, h);
}

void drawChannel(String label, float value, int channelColor, float x, float y, float w, float h) {
  fill(245);
  textAlign(LEFT, CENTER);
  textSize(18);
  text(label, x - 30, y + h / 2.0);

  noStroke();
  fill(28, 30, 36);
  rect(x, y, w, h, 6);

  fill(channelColor);
  rect(x, y, w * constrain(value, 0.0, 1.0), h, 6);

  fill(245, 190);
  textAlign(RIGHT, CENTER);
  text(nf(value, 1, 3), x + w, y - 13);
}

void drawHistory() {
  float chipSize = 42;
  float x = 72;
  float y = height - 64;

  fill(245);
  textAlign(LEFT, CENTER);
  textSize(18);
  text("recent", x, y - 38);

  for (int i = 0; i < history.size(); i++) {
    ColorChip chip = history.get(i);
    float alpha = map(i, 0, max(1, history.size() - 1), 255, 70);

    noStroke();
    fill(red(chip.c), green(chip.c), blue(chip.c), alpha);
    rect(x + i * (chipSize + 10), y, chipSize, chipSize, 10);

    noFill();
    stroke(255, alpha * 0.7);
    strokeWeight(1.5);
    rect(x + i * (chipSize + 10), y, chipSize, chipSize, 10);
  }
}

void drawHud() {
  fill(245);
  textAlign(LEFT, TOP);
  textSize(22);
  text("APDS9960 color studio", 40, 36);

  textSize(18);
  text("Status: " + statusText(), 40, 72);
  text("Packets: " + packetCount, 40, 102);
  text("OSC: " + rgbPath, 40, 132);
}

String statusText() {
  if (!receivedData) {
    return "waiting for OSC";
  }

  if (millis() - lastPacketMs > 1600) {
    return "stale";
  }

  return "receiving OSC";
}

String rgbLabel() {
  return "rgb " + nf(redValue, 1, 3) + "  " +
         nf(greenValue, 1, 3) + "  " +
         nf(blueValue, 1, 3);
}

int sensedColor(float r, float g, float b) {
  return color(255 * constrain(r, 0.0, 1.0),
               255 * constrain(g, 0.0, 1.0),
               255 * constrain(b, 0.0, 1.0));
}

int contrastColor(int c) {
  float luma = red(c) * 0.2126 + green(c) * 0.7152 + blue(c) * 0.0722;
  return luma > 140 ? color(12, 12, 16) : color(245);
}

void oscEvent(OscMessage message) {
  if (!message.checkAddrPattern(rgbPath)) {
    return;
  }

  if (message.typetag().length() < 3) {
    return;
  }

  redValue = readChannel(message, 0);
  greenValue = readChannel(message, 1);
  blueValue = readChannel(message, 2);

  receivedData = true;
  lastPacketMs = millis();
  packetCount++;

  if (history.size() == 0 || colorDistance(history.get(0).c, sensedColor(redValue, greenValue, blueValue)) > 18) {
    history.add(0, new ColorChip(sensedColor(redValue, greenValue, blueValue)));
  }

  while (history.size() > 18) {
    history.remove(history.size() - 1);
  }
}

float readChannel(OscMessage message, int index) {
  char tag = message.typetag().charAt(index);

  if (tag == 'f') {
    return constrain(message.get(index).floatValue(), 0.0, 1.0);
  }

  if (tag == 'i') {
    return constrain(message.get(index).intValue() / 255.0, 0.0, 1.0);
  }

  return 0.0;
}

float colorDistance(int a, int b) {
  float dr = red(a) - red(b);
  float dg = green(a) - green(b);
  float db = blue(a) - blue(b);

  return sqrt(dr * dr + dg * dg + db * db);
}

class ColorChip {
  int c;

  ColorChip(int c) {
    this.c = c;
  }
}
