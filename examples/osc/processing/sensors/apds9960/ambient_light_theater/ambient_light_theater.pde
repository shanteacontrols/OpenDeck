/*
  OpenDeck APDS9960 Ambient Light Theater

  Receives OpenDeck OSC ambient light data:
    /opendeck/sensors/apds9960/ambient_light  f
*/

import oscP5.*;
import netP5.*;

OscP5 oscP5;

int oscListenPort = 9000;
String ambientPath = "/opendeck/sensors/apds9960/ambient_light";

float ambient = 0.0;
float visualAmbient = 0.0;
int lastPacketMs = 0;
int packetCount = 0;
boolean receivedData = false;

ArrayList<Dust> dust = new ArrayList<Dust>();

void setup() {
  size(1200, 900, P2D);
  surface.setTitle("OpenDeck APDS9960 Ambient Light Theater");

  oscP5 = new OscP5(this, oscListenPort);
  textFont(createFont("SansSerif", 18));

  for (int i = 0; i < 220; i++) {
    dust.add(new Dust());
  }
}

void draw() {
  visualAmbient += (ambient - visualAmbient) * 0.08;

  drawRoom();
  drawWindowLight();
  drawBlinds();
  drawDust();
  drawMeter();
  drawHud();
}

void drawRoom() {
  int topColor = lerpColor(color(4, 6, 12), color(24, 30, 42), visualAmbient);
  int bottomColor = lerpColor(color(10, 8, 13), color(70, 56, 40), visualAmbient);

  noStroke();
  for (int y = 0; y < height; y++) {
    float t = y / float(height - 1);
    stroke(lerpColor(topColor, bottomColor, t));
    line(0, y, width, y);
  }

  noStroke();
  fill(0, 80 - visualAmbient * 48);
  rect(0, height * 0.72, width, height * 0.28);
}

void drawWindowLight() {
  float cx = width / 2.0;
  float cy = height / 2.0 - 30;
  float glow = 260 + visualAmbient * 520;
  float beamAlpha = 18 + visualAmbient * 96;

  noStroke();
  for (int i = 9; i >= 1; i--) {
    float t = i / 9.0;
    fill(255, 220, 130, beamAlpha * 0.18 * t);
    ellipse(cx, cy, glow * t, glow * 0.72 * t);
  }

  fill(255, 220, 120, 20 + visualAmbient * 85);
  quad(cx - 142, cy - 144, cx + 142, cy - 144, width - 70, height, 70, height);

  fill(255, 235, 180, 30 + visualAmbient * 150);
  rect(cx - 150, cy - 150, 300, 300, 8);

  stroke(255, 245, 210, 80 + visualAmbient * 120);
  strokeWeight(4);
  noFill();
  rect(cx - 150, cy - 150, 300, 300, 8);

  stroke(255, 245, 210, 70 + visualAmbient * 100);
  line(cx, cy - 150, cx, cy + 150);
  line(cx - 150, cy, cx + 150, cy);
}

void drawBlinds() {
  float cx = width / 2.0;
  float cy = height / 2.0 - 30;
  float open = visualAmbient;
  float slatHeight = 16;

  noStroke();

  for (int i = 0; i < 12; i++) {
    float y = cy - 142 + i * 25;
    float tilt = map(open, 0.0, 1.0, 0, 18);

    fill(18, 22, 30, 210 - visualAmbient * 90);
    quad(cx - 156, y,
         cx + 156, y,
         cx + 150 - tilt, y + slatHeight,
         cx - 150 + tilt, y + slatHeight);
  }
}

void drawDust() {
  for (Dust particle : dust) {
    particle.update(visualAmbient);
    particle.draw(visualAmbient);
  }
}

void drawMeter() {
  float x = 54;
  float y = height - 90;
  float w = width - 108;
  float h = 24;

  noStroke();
  fill(28, 30, 36);
  rect(x, y, w, h, 6);

  fill(255, 214, 96);
  rect(x, y, w * visualAmbient, h, 6);

  noFill();
  stroke(255, 180);
  strokeWeight(2);
  rect(x, y, w, h, 6);

  fill(245);
  textAlign(LEFT, BOTTOM);
  textSize(18);
  text("ambient light " + nf(ambient, 1, 3), x, y - 10);
}

void drawHud() {
  fill(245);
  textAlign(LEFT, TOP);
  textSize(22);
  text("APDS9960 ambient light theater", 40, 36);

  textSize(18);
  text("Status: " + statusText(), 40, 72);
  text("Packets: " + packetCount, 40, 102);
  text("OSC: " + ambientPath, 40, 132);
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

void oscEvent(OscMessage message) {
  if (!message.checkAddrPattern(ambientPath)) {
    return;
  }

  if (message.typetag().length() < 1) {
    return;
  }

  if (message.typetag().charAt(0) == 'f') {
    ambient = constrain(message.get(0).floatValue(), 0.0, 1.0);
  } else if (message.typetag().charAt(0) == 'i') {
    ambient = constrain(message.get(0).intValue() / 255.0, 0.0, 1.0);
  } else {
    return;
  }

  receivedData = true;
  lastPacketMs = millis();
  packetCount++;
}

class Dust {
  float x;
  float y;
  float size;
  float speed;
  float phase;

  Dust() {
    reset();
    y = random(height);
  }

  void reset() {
    x = random(width);
    y = random(-80, height + 80);
    size = random(2, 8);
    speed = random(0.18, 0.85);
    phase = random(TWO_PI);
  }

  void update(float light) {
    y -= speed * (0.25 + light * 1.7);
    x += sin(frameCount * 0.018 + phase) * (0.1 + light * 0.42);

    if (y < -80 || x < -80 || x > width + 80) {
      reset();
      y = height + random(20, 120);
    }
  }

  void draw(float light) {
    float alpha = (18 + light * 130) * map(y, height, 0, 0.35, 1.0);

    noStroke();
    fill(255, 235, 170, alpha);
    ellipse(x, y, size + light * 4, size + light * 4);
  }
}
