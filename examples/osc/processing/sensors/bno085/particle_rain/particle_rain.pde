/*
  OpenDeck BNO085 Particle Rain

  Receives OpenDeck OSC IMU gravity data:
    /opendeck/sensors/bno085/gravity  fff

  The BNO085 gravity vector drives particle acceleration directly.
*/

import oscP5.*;
import netP5.*;

class Particle {
  PVector position = new PVector();
  PVector velocity = new PVector();
  float radius = 4.0;
  color tint = color(255);

  Particle() {
    reset(true);
  }

  void reset(boolean randomHeight) {
    position.x = random(-boxHalfWidth, boxHalfWidth);
    position.y = randomHeight ? random(-boxHalfHeight, boxHalfHeight) : -boxHalfHeight;
    position.z = random(-boxHalfDepth, boxHalfDepth);
    velocity.x = random(-0.35, 0.35);
    velocity.y = random(-0.35, 0.35);
    velocity.z = random(-0.35, 0.35);
    radius = random(2.5, 5.5);

    float hue = random(0.0, 1.0);
    tint = lerpColor(color(40, 185, 255), color(255, 195, 40), hue);
  }

  void resetFromGravity(PVector acceleration) {
    PVector source = acceleration.copy();

    if (source.mag() < 0.0001) {
      reset(true);
      return;
    }

    source.normalize();
    source.mult(-1);

    position.x = source.x * boxHalfWidth + random(-80, 80);
    position.y = source.y * boxHalfHeight + random(-80, 80);
    position.z = source.z * boxHalfDepth + random(-80, 80);

    position.x = constrain(position.x, -boxHalfWidth, boxHalfWidth);
    position.y = constrain(position.y, -boxHalfHeight, boxHalfHeight);
    position.z = constrain(position.z, -boxHalfDepth, boxHalfDepth);

    velocity.x = random(-0.45, 0.45);
    velocity.y = random(-0.45, 0.45);
    velocity.z = random(-0.45, 0.45);
  }

  void update(PVector acceleration) {
    velocity.add(acceleration);
    velocity.mult(0.992);
    velocity.limit(18.0);
    position.add(velocity);

    if (outsideBounds(36)) {
      resetFromGravity(acceleration);
    }
  }

  boolean outsideBounds(float margin) {
    return position.x < -boxHalfWidth - margin ||
           position.x > boxHalfWidth + margin ||
           position.y < -boxHalfHeight - margin ||
           position.y > boxHalfHeight + margin ||
           position.z < -boxHalfDepth - margin ||
           position.z > boxHalfDepth + margin;
  }

  void draw() {
    pushMatrix();
    translate(position.x, position.y, position.z);
    noStroke();
    fill(tint);
    specular(220);
    shininess(22);
    sphere(radius);
    popMatrix();
  }
}

OscP5 oscP5;

int oscListenPort = 9000;
String gravityPath = "/opendeck/sensors/bno085/gravity";

float gravityX = 0.0;
float gravityY = 0.0;
float gravityZ = 0.0;

boolean receivedData = false;
boolean paused = false;
boolean drawTrails = false;

int lastPacketMs = 0;

float viewRotX = -0.78;
float viewRotY = 0.0;
float viewRotZ = -0.55;
float zoom = 1.0;

float boxHalfWidth = 360;
float boxHalfHeight = 245;
float boxHalfDepth = 220;

ArrayList<Particle> particles = new ArrayList<Particle>();
int particleCount = 120;

void setup() {
  size(1200, 900, P3D);
  surface.setTitle("OpenDeck BNO085 Particle Rain");

  oscP5 = new OscP5(this, oscListenPort);
  textFont(createFont("SansSerif", 18));

  for (int i = 0; i < particleCount; i++) {
    particles.add(new Particle());
  }
}

void draw() {
  if (drawTrails) {
    fill(0, 36);
    noStroke();
    rect(0, 0, width, height);
  } else {
    background(4);
  }

  PVector acceleration = gravityAcceleration();

  if (!paused) {
    for (Particle particle : particles) {
      particle.update(acceleration);
    }
  }

  lights();
  ambientLight(60, 60, 72);
  directionalLight(235, 230, 215, -0.42, 0.48, -0.7);
  directionalLight(50, 120, 220, 0.75, -0.25, 0.2);

  pushMatrix();
  translate(width / 2, height / 2 + 80, -60);
  scale(zoom);
  rotateX(viewRotX);
  rotateZ(viewRotZ);
  rotateY(viewRotY);

  drawBox();
  drawGravityRibbon(acceleration);

  for (Particle particle : particles) {
    particle.draw();
  }

  popMatrix();

  drawHud(acceleration);
}

PVector gravityAcceleration() {
  PVector gravity = new PVector(gravityX, gravityY, gravityZ);

  if (gravity.mag() < 0.0001) {
    return new PVector(0, 0.26, 0);
  }

  gravity.normalize();

  return new PVector(gravity.x * 0.34, -gravity.y * 0.34, -gravity.z * 0.34);
}

void drawBox() {
  noFill();
  stroke(80, 95, 115, 135);
  strokeWeight(1.4);
  box(boxHalfWidth * 2, boxHalfHeight * 2, boxHalfDepth * 2);

  stroke(55, 62, 76, 90);
  strokeWeight(1);

  for (int i = -3; i <= 3; i++) {
    float x = i * boxHalfWidth / 3.0;
    line(x, -boxHalfHeight, -boxHalfDepth, x, boxHalfHeight, -boxHalfDepth);
    line(x, -boxHalfHeight, boxHalfDepth, x, boxHalfHeight, boxHalfDepth);
  }

  for (int i = -2; i <= 2; i++) {
    float z = i * boxHalfDepth / 2.0;
    line(-boxHalfWidth, boxHalfHeight, z, boxHalfWidth, boxHalfHeight, z);
    line(-boxHalfWidth, -boxHalfHeight, z, boxHalfWidth, -boxHalfHeight, z);
  }
}

void drawGravityRibbon(PVector acceleration) {
  PVector arrow = acceleration.copy();

  if (arrow.mag() < 0.0001) {
    return;
  }

  arrow.normalize();
  arrow.mult(145);

  strokeWeight(7);
  stroke(255, 215, 45, 215);
  line(0, 0, 0, arrow.x, arrow.y, arrow.z);

  pushMatrix();
  translate(arrow.x, arrow.y, arrow.z);
  noStroke();
  fill(255, 230, 80);
  sphere(18);
  popMatrix();
}

void drawHud(PVector acceleration) {
  hint(DISABLE_DEPTH_TEST);
  camera();
  noLights();

  fill(238);
  textAlign(LEFT, TOP);
  textSize(20);

  String status = receivedData ? "receiving OSC" : "waiting for OSC on port " + oscListenPort;

  text("BNO085 particle rain", 38, 34);
  text("Status: " + status, 38, 66);
  text("Gravity: x " + nf(gravityX, 0, 3) + "  y " + nf(gravityY, 0, 3) + "  z " + nf(gravityZ, 0, 3), 38, 98);
  text("Particles: " + particleCount + "   Mode: " + (drawTrails ? "trails" : "clean"), 38, 130);
  text("Keys: space pause, c clear, t trails, [ ] particles, r reset view, +/- zoom", 38, height - 52);

  drawTiltDisk(width - 170, 118);

  if (millis() - lastPacketMs > 2000) {
    fill(255, 190, 40);
    text("Enable BNO085 Gravity output in OpenDeck.", 38, 164);
  }

  hint(ENABLE_DEPTH_TEST);
}

void drawTiltDisk(float x, float y) {
  PVector gravity = new PVector(gravityX, gravityY, gravityZ);
  float radius = 72;

  noFill();
  stroke(150);
  strokeWeight(2);
  ellipse(x, y, radius * 2, radius * 2);

  stroke(80);
  line(x - radius, y, x + radius, y);
  line(x, y - radius, x, y + radius);

  if (gravity.mag() > 0.0001) {
    gravity.normalize();
  }

  float px = x + constrain(gravity.x, -1, 1) * radius;
  float py = y - constrain(gravity.y, -1, 1) * radius;

  noStroke();
  fill(255, 215, 45);
  ellipse(px, py, 18, 18);
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
  if (key == ' ') {
    paused = !paused;
  } else if (key == 'c' || key == 'C') {
    background(4);
    for (Particle particle : particles) {
      particle.reset(true);
    }
  } else if (key == 't' || key == 'T') {
    drawTrails = !drawTrails;
    background(4);
  } else if (key == '[') {
    setParticleCount(max(30, particleCount - 30));
  } else if (key == ']') {
    setParticleCount(min(360, particleCount + 30));
  } else if (key == 'r' || key == 'R') {
    viewRotX = -0.78;
    viewRotY = 0.0;
    viewRotZ = -0.55;
    zoom = 1.0;
  } else if (key == '+') {
    zoom = min(1.8, zoom + 0.08);
  } else if (key == '-') {
    zoom = max(0.55, zoom - 0.08);
  }
}

void setParticleCount(int count) {
  particleCount = count;

  while (particles.size() > particleCount) {
    particles.remove(particles.size() - 1);
  }

  while (particles.size() < particleCount) {
    particles.add(new Particle());
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
