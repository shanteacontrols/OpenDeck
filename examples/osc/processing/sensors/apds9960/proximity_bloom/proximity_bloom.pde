/*
  OpenDeck APDS9960 Proximity Bloom

  Receives OpenDeck OSC proximity data:
    /opendeck/sensors/apds9960/proximity  i
*/

import oscP5.*;
import netP5.*;

OscP5 oscP5;

int oscListenPort = 9000;
String proximityPath = "/opendeck/sensors/apds9960/proximity";

int proximity = 0;
int lastPacketMs = 0;
int packetCount = 0;
boolean receivedData = false;

float visualProximity = 0.0;
float pulsePhase = 0.0;
float idlePhase = 0.0;

ArrayList<Spark> sparks = new ArrayList<Spark>();
int maxSparks = 180;

void setup() {
  size(1200, 900, P2D);
  surface.setTitle("OpenDeck APDS9960 Proximity Bloom");

  oscP5 = new OscP5(this, oscListenPort);
  textFont(createFont("SansSerif", 18));

  for (int i = 0; i < maxSparks; i++) {
    sparks.add(new Spark());
  }
}

void draw() {
  background(5, 6, 8);

  float target = proximity / 255.0;
  visualProximity += (target - visualProximity) * 0.18;
  pulsePhase += 0.025 + visualProximity * 0.06;
  idlePhase += 0.012;

  updateSparks();

  pushMatrix();
  translate(width / 2.0, height / 2.0 + 30);
  drawGlow();
  drawBloom();
  drawCore();
  drawSparks();
  popMatrix();

  drawMeter();
  drawHud();
}

void updateSparks() {
  int active = int(map(visualProximity, 0.0, 1.0, 12, maxSparks));

  for (int i = 0; i < sparks.size(); i++) {
    Spark spark = sparks.get(i);

    if (i < active) {
      spark.update(visualProximity);
    } else {
      spark.sleep();
    }
  }
}

void drawGlow() {
  noStroke();

  float glow = 120 + visualProximity * 380;

  for (int i = 7; i >= 1; i--) {
    float amount = i / 7.0;
    fill(255, 150 + 70 * visualProximity, 25, 7 + visualProximity * 12);
    ellipse(0, 0, glow * amount, glow * amount);
  }
}

void drawBloom() {
  int petals = 18;
  float baseRadius = 80 + visualProximity * 250;
  float petalLength = 70 + visualProximity * 270;
  float petalWidth = 28 + visualProximity * 58;
  float wobble = sin(pulsePhase) * (6 + visualProximity * 24);

  noStroke();

  for (int i = 0; i < petals; i++) {
    float angle = TWO_PI * i / petals;
    float localPulse = sin(pulsePhase * 1.7 + i * 0.62);
    float radius = baseRadius + localPulse * wobble;
    float hueAmount = i / float(petals);

    pushMatrix();
    rotate(angle + sin(idlePhase + i) * 0.025);
    translate(radius * 0.42, 0);

    colorMode(HSB, 360, 100, 100, 100);
    fill(35 + hueAmount * 42, 90, 80 + visualProximity * 20, 52 + visualProximity * 42);
    colorMode(RGB, 255);

    ellipse(petalLength * 0.35, 0, petalLength, petalWidth);

    popMatrix();
  }

  noFill();
  stroke(255, 210, 90, 80 + visualProximity * 120);
  strokeWeight(2 + visualProximity * 6);
  ellipse(0, 0, baseRadius * 1.04, baseRadius * 1.04);
}

void drawCore() {
  float core = 54 + visualProximity * 155 + sin(pulsePhase * 2.5) * visualProximity * 16;

  noStroke();
  fill(255, 235, 120, 220);
  ellipse(0, 0, core, core);

  fill(255, 255, 245, 160);
  ellipse(-core * 0.18, -core * 0.22, core * 0.32, core * 0.32);
}

void drawSparks() {
  for (Spark spark : sparks) {
    spark.draw();
  }
}

void drawMeter() {
  float x = 54;
  float y = height - 90;
  float w = width - 108;
  float h = 24;

  noStroke();
  fill(32);
  rect(x, y, w, h, 6);

  fill(255, 170, 35);
  rect(x, y, w * visualProximity, h, 6);

  noFill();
  stroke(255, 170);
  strokeWeight(2);
  rect(x, y, w, h, 6);

  fill(235);
  textAlign(LEFT, BOTTOM);
  textSize(18);
  text("proximity " + proximity + " / 255", x, y - 10);
}

void drawHud() {
  fill(245);
  textAlign(LEFT, TOP);
  textSize(22);
  text("APDS9960 proximity bloom", 40, 38);

  textSize(18);
  text("Status: " + statusText(), 40, 72);
  text("Packets: " + packetCount, 40, 102);
  text("OSC: " + proximityPath, 40, 132);
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
  if (!message.checkAddrPattern(proximityPath)) {
    return;
  }

  if (message.typetag().length() < 1) {
    return;
  }

  if (message.typetag().charAt(0) == 'i') {
    proximity = constrain(message.get(0).intValue(), 0, 255);
  } else if (message.typetag().charAt(0) == 'f') {
    proximity = constrain(round(message.get(0).floatValue()), 0, 255);
  } else {
    return;
  }

  receivedData = true;
  lastPacketMs = millis();
  packetCount++;
}

class Spark {
  float angle;
  float radius;
  float speed;
  float size;
  float alpha;

  Spark() {
    reset();
  }

  void reset() {
    angle = random(TWO_PI);
    radius = random(70, 420);
    speed = random(0.004, 0.018) * (random(1) < 0.5 ? -1 : 1);
    size = random(4, 18);
    alpha = random(60, 190);
  }

  void update(float energy) {
    angle += speed * (0.7 + energy * 3.0);
    radius += sin(frameCount * 0.018 + angle * 2.0) * (0.25 + energy * 1.8);

    float minRadius = 70 + energy * 45;
    float maxRadius = 210 + energy * 390;

    if (radius < minRadius || radius > maxRadius) {
      reset();
      radius = random(minRadius, maxRadius);
    }
  }

  void sleep() {
    alpha *= 0.94;
  }

  void draw() {
    if (alpha < 4) {
      return;
    }

    float x = cos(angle) * radius;
    float y = sin(angle) * radius;
    float warm = map(radius, 60, 620, 1.0, 0.0);

    noStroke();
    fill(255, 150 + 95 * warm, 35 + 120 * (1.0 - warm), alpha);
    ellipse(x, y, size, size);
  }
}
