/*
  OpenDeck BNO085 Gravity Visualizer

  Receives OpenDeck OSC IMU gravity data:
    /opendeck/sensors/bno085/gravity  fff

  The vector is shown directly, without Processing-side smoothing.
*/

import oscP5.*;
import netP5.*;

OscP5 oscP5;

int oscListenPort = 9000;
String gravityPath = "/opendeck/sensors/bno085/gravity";

float gravityX = 0.0;
float gravityY = 0.0;
float gravityZ = 0.0;

boolean receivedData = false;
boolean showTrail = true;

int lastPacketMs = 0;

float viewRotX = -0.68;
float viewRotY = 0.0;
float viewRotZ = -0.62;
float zoom = 1.0;

ArrayList<PVector> trail = new ArrayList<PVector>();
int maxTrailPoints = 220;

void setup() {
  size(1200, 900, P3D);
  surface.setTitle("OpenDeck BNO085 Gravity Visualizer");

  oscP5 = new OscP5(this, oscListenPort);
  textFont(createFont("SansSerif", 18));
}

void draw() {
  background(5);
  updateTrail();

  lights();
  ambientLight(70, 70, 78);
  directionalLight(230, 230, 220, -0.45, 0.45, -0.7);
  directionalLight(70, 130, 190, 0.6, -0.4, 0.25);

  pushMatrix();
  translate(width / 2, height / 2 + 70, -40);
  scale(zoom);
  rotateX(viewRotX);
  rotateZ(viewRotZ);
  rotateY(viewRotY);

  drawReferenceSphere();
  drawAxes();
  drawTrail();
  drawGravityVector();

  popMatrix();

  drawHud();
}

void updateTrail() {
  if (!showTrail || !receivedData) {
    return;
  }

  trail.add(gravityPoint(210));

  while (trail.size() > maxTrailPoints) {
    trail.remove(0);
  }
}

PVector gravityPoint(float radius) {
  PVector gravity = new PVector(gravityX, gravityY, gravityZ);

  if (gravity.mag() < 0.0001) {
    return new PVector();
  }

  gravity.normalize();

  return new PVector(gravity.x * radius, -gravity.y * radius, -gravity.z * radius);
}

void drawReferenceSphere() {
  noFill();
  strokeWeight(1);
  stroke(80, 90, 110, 100);

  int rings = 9;
  float radius = 210;

  for (int i = 1; i < rings; i++) {
    float t = map(i, 0, rings, -HALF_PI, HALF_PI);
    float y = sin(t) * radius;
    float r = cos(t) * radius;

    beginShape();
    for (int a = 0; a <= 72; a++) {
      float angle = TWO_PI * a / 72.0;
      vertex(cos(angle) * r, y, sin(angle) * r);
    }
    endShape();
  }

  for (int i = 0; i < 8; i++) {
    pushMatrix();
    rotateY(TWO_PI * i / 8.0);
    beginShape();
    for (int a = -36; a <= 36; a++) {
      float angle = HALF_PI * a / 36.0;
      vertex(cos(angle) * radius, sin(angle) * radius, 0);
    }
    endShape();
    popMatrix();
  }
}

void drawAxes() {
  strokeWeight(3);

  stroke(255, 70, 70, 160);
  line(-260, 0, 0, 260, 0, 0);

  stroke(70, 220, 130, 160);
  line(0, -260, 0, 0, 260, 0);

  stroke(80, 150, 255, 160);
  line(0, 0, -260, 0, 0, 260);
}

void drawTrail() {
  if (!showTrail || trail.size() < 2) {
    return;
  }

  noFill();
  strokeWeight(3);

  for (int i = 1; i < trail.size(); i++) {
    float alpha = map(i, 1, trail.size() - 1, 18, 170);
    stroke(255, 190, 40, alpha);
    PVector a = trail.get(i - 1);
    PVector b = trail.get(i);
    line(a.x, a.y, a.z, b.x, b.y, b.z);
  }
}

void drawGravityVector() {
  PVector end = gravityPoint(230);

  strokeWeight(8);
  stroke(255, 210, 35);
  line(0, 0, 0, end.x, end.y, end.z);

  pushMatrix();
  translate(end.x, end.y, end.z);
  noStroke();
  fill(255, 230, 50);
  specular(180);
  shininess(18);
  sphere(26);
  popMatrix();

  pushMatrix();
  PVector projection = new PVector(end.x, end.y, 0);
  strokeWeight(2);
  stroke(255, 255, 255, 90);
  line(end.x, end.y, end.z, projection.x, projection.y, projection.z);
  popMatrix();

  noStroke();
  fill(255, 255, 255, 210);
  sphere(7);
}

void drawHud() {
  hint(DISABLE_DEPTH_TEST);
  camera();
  noLights();

  fill(238);
  textAlign(LEFT, TOP);
  textSize(20);

  String status = receivedData ? "receiving OSC" : "waiting for OSC on port " + oscListenPort;
  float magnitude = sqrt((gravityX * gravityX) + (gravityY * gravityY) + (gravityZ * gravityZ));

  text("BNO085 gravity visualizer", 38, 34);
  text("Status: " + status, 38, 66);
  text("Gravity: x " + nf(gravityX, 0, 3) + "  y " + nf(gravityY, 0, 3) + "  z " + nf(gravityZ, 0, 3), 38, 98);
  text("Magnitude: " + nf(magnitude, 0, 3), 38, 130);
  text("Keys: t trail, r reset view, +/- zoom", 38, height - 52);

  drawComponentMeter(width - 280, 50, "X", gravityX, color(255, 70, 70));
  drawComponentMeter(width - 200, 50, "Y", gravityY, color(70, 220, 130));
  drawComponentMeter(width - 120, 50, "Z", gravityZ, color(80, 150, 255));

  if (millis() - lastPacketMs > 2000) {
    fill(255, 190, 40);
    text("Enable BNO085 Gravity output in OpenDeck.", 38, 164);
  }

  hint(ENABLE_DEPTH_TEST);
}

void drawComponentMeter(float x, float y, String label, float value, color c) {
  float h = 160;
  float center = y + h / 2.0;
  float amount = constrain(value, -1.2, 1.2) / 1.2;

  noFill();
  stroke(90);
  strokeWeight(2);
  rect(x, y, 42, h, 8);

  stroke(130);
  line(x, center, x + 42, center);

  noStroke();
  fill(c);

  if (amount >= 0) {
    rect(x + 6, center - (amount * h / 2.0), 30, amount * h / 2.0, 5);
  } else {
    rect(x + 6, center, 30, -amount * h / 2.0, 5);
  }

  fill(235);
  textAlign(CENTER, TOP);
  textSize(16);
  text(label, x + 21, y + h + 10);
}

void oscEvent(OscMessage message) {
  if (message.addrPattern().equals(gravityPath) && message.checkTypetag("fff")) {
    gravityX = message.get(0).floatValue();
    gravityY = message.get(1).floatValue();
    gravityZ = message.get(2).floatValue();

    receivedData = true;
    lastPacketMs = millis();
  }
}

void keyPressed() {
  if (key == 't' || key == 'T') {
    showTrail = !showTrail;
    trail.clear();
  } else if (key == 'r' || key == 'R') {
    viewRotX = -0.68;
    viewRotY = 0.0;
    viewRotZ = -0.62;
    zoom = 1.0;
  } else if (key == '+') {
    zoom = min(1.8, zoom + 0.08);
  } else if (key == '-') {
    zoom = max(0.55, zoom - 0.08);
  }
}

void mouseDragged() {
  if (mouseButton == LEFT) {
    viewRotZ += (mouseX - pmouseX) * 0.01;
    viewRotX += (mouseY - pmouseY) * 0.01;
  } else if (mouseButton == RIGHT) {
    viewRotY += (mouseX - pmouseX) * 0.01;
  }
}
