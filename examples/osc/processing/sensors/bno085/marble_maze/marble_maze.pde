/*
  OpenDeck BNO085 Marble Maze

  Receives OpenDeck OSC IMU gravity data:
    /opendeck/sensors/bno085/gravity  fff

  Tilt the BNO085 to roll the marble through the maze.
*/

import oscP5.*;
import netP5.*;

class Wall {
  float x;
  float y;
  float w;
  float h;

  Wall(float x, float y, float w, float h) {
    this.x = x;
    this.y = y;
    this.w = w;
    this.h = h;
  }

  void draw() {
    noStroke();
    fill(34, 92, 120);
    rect(x, y, w, h, 8);

    fill(60, 145, 185, 70);
    rect(x + 3, y + 3, w - 6, 5, 4);
  }

  boolean collides(PVector p, float r) {
    float closestX = constrain(p.x, x, x + w);
    float closestY = constrain(p.y, y, y + h);
    float dx = p.x - closestX;
    float dy = p.y - closestY;

    return dx * dx + dy * dy < r * r;
  }

  void resolve(PVector p, PVector v, float r) {
    float left = abs((x - r) - p.x);
    float right = abs((x + w + r) - p.x);
    float top = abs((y - r) - p.y);
    float bottom = abs((y + h + r) - p.y);
    float side = min(min(left, right), min(top, bottom));

    if (side == left) {
      p.x = x - r;
      v.x = min(0, v.x) * -0.42;
    } else if (side == right) {
      p.x = x + w + r;
      v.x = max(0, v.x) * -0.42;
    } else if (side == top) {
      p.y = y - r;
      v.y = min(0, v.y) * -0.42;
    } else {
      p.y = y + h + r;
      v.y = max(0, v.y) * -0.42;
    }
  }
}

OscP5 oscP5;

int oscListenPort = 9000;
String gravityPath = "/opendeck/sensors/bno085/gravity";

float gravityX = 0.0;
float gravityY = 0.0;
float gravityZ = 0.0;

boolean receivedData = false;
boolean won = false;

int lastPacketMs = 0;
int winMs = 0;

float boardX = 110;
float boardY = 92;
float boardW = 980;
float boardH = 690;

float marbleRadius = 17;
PVector marble = new PVector();
PVector velocity = new PVector();

PVector start = new PVector();
PVector goal = new PVector();

ArrayList<Wall> walls = new ArrayList<Wall>();

void setup() {
  size(1200, 900, P2D);
  surface.setTitle("OpenDeck BNO085 Marble Maze");

  oscP5 = new OscP5(this, oscListenPort);
  textFont(createFont("SansSerif", 18));

  buildMaze();
  resetMarble();
}

void draw() {
  background(5);

  if (!won) {
    updateMarble();
  }

  drawBoard();
  drawHud();

  if (won && millis() - winMs > 1500) {
    resetMarble();
  }
}

void buildMaze() {
  walls.clear();

  start.set(boardX + 58, boardY + 58);
  goal.set(boardX + boardW - 70, boardY + boardH - 70);

  walls.add(new Wall(boardX, boardY, boardW, 18));
  walls.add(new Wall(boardX, boardY + boardH - 18, boardW, 18));
  walls.add(new Wall(boardX, boardY, 18, boardH));
  walls.add(new Wall(boardX + boardW - 18, boardY, 18, boardH));

  walls.add(new Wall(boardX + 105, boardY + 90, 18, 470));
  walls.add(new Wall(boardX + 105, boardY + 90, 220, 18));
  walls.add(new Wall(boardX + 228, boardY + 185, 18, 380));
  walls.add(new Wall(boardX + 228, boardY + 545, 265, 18));

  walls.add(new Wall(boardX + 340, boardY + 18, 18, 310));
  walls.add(new Wall(boardX + 340, boardY + 310, 225, 18));
  walls.add(new Wall(boardX + 470, boardY + 112, 18, 200));
  walls.add(new Wall(boardX + 470, boardY + 112, 270, 18));

  walls.add(new Wall(boardX + 590, boardY + 220, 18, 360));
  walls.add(new Wall(boardX + 430, boardY + 420, 178, 18));
  walls.add(new Wall(boardX + 720, boardY + 130, 18, 360));
  walls.add(new Wall(boardX + 720, boardY + 472, 190, 18));

  walls.add(new Wall(boardX + 830, boardY + 250, 18, 285));
  walls.add(new Wall(boardX + 735, boardY + 615, 18, 58));
  walls.add(new Wall(boardX + 735, boardY + 615, 180, 18));
}

void resetMarble() {
  marble.set(start);
  velocity.set(0, 0);
  won = false;
}

void updateMarble() {
  PVector accel = boardAcceleration();

  velocity.add(accel);
  velocity.mult(0.972);
  velocity.limit(14);

  marble.add(velocity);

  constrainToBoard();

  for (Wall wall : walls) {
    if (wall.collides(marble, marbleRadius)) {
      wall.resolve(marble, velocity, marbleRadius);
    }
  }

  if (dist(marble.x, marble.y, goal.x, goal.y) < 32) {
    won = true;
    winMs = millis();
  }
}

PVector boardAcceleration() {
  PVector gravity = new PVector(gravityX, gravityY);

  if (gravity.mag() < 0.0001) {
    return new PVector();
  }

  gravity.limit(1.2);

  return new PVector(gravity.x * 0.42, -gravity.y * 0.42);
}

void constrainToBoard() {
  if (marble.x < boardX + marbleRadius) {
    marble.x = boardX + marbleRadius;
    velocity.x *= -0.5;
  } else if (marble.x > boardX + boardW - marbleRadius) {
    marble.x = boardX + boardW - marbleRadius;
    velocity.x *= -0.5;
  }

  if (marble.y < boardY + marbleRadius) {
    marble.y = boardY + marbleRadius;
    velocity.y *= -0.5;
  } else if (marble.y > boardY + boardH - marbleRadius) {
    marble.y = boardY + boardH - marbleRadius;
    velocity.y *= -0.5;
  }
}

void drawBoard() {
  noStroke();
  fill(13, 18, 22);
  rect(boardX - 10, boardY - 10, boardW + 20, boardH + 20, 14);

  fill(10, 32, 42);
  rect(boardX, boardY, boardW, boardH, 10);

  drawGoal();

  for (Wall wall : walls) {
    wall.draw();
  }

  drawMarble();
}

void drawGoal() {
  noStroke();
  fill(255, 185, 32, 70);
  ellipse(goal.x, goal.y, 88, 88);
  fill(255, 190, 40);
  ellipse(goal.x, goal.y, 48, 48);
  fill(20);
  ellipse(goal.x, goal.y, 18, 18);

  fill(80, 220, 150, 160);
  ellipse(start.x, start.y, 42, 42);
}

void drawMarble() {
  noStroke();
  fill(0, 0, 0, 90);
  ellipse(marble.x + 8, marble.y + 9, marbleRadius * 2.1, marbleRadius * 1.35);

  fill(235, 248, 255);
  ellipse(marble.x, marble.y, marbleRadius * 2, marbleRadius * 2);

  fill(90, 190, 255);
  ellipse(marble.x - 5, marble.y - 5, marbleRadius * 0.9, marbleRadius * 0.9);

  fill(255, 255, 255, 210);
  ellipse(marble.x - 8, marble.y - 10, marbleRadius * 0.38, marbleRadius * 0.38);
}

void drawHud() {
  fill(238);
  textAlign(LEFT, TOP);
  textSize(20);

  String status = receivedData ? "receiving OSC" : "waiting for OSC on port " + oscListenPort;

  text("BNO085 marble maze", 40, 28);
  text("Status: " + status, 40, 60);
  text("Gravity: x " + nf(gravityX, 0, 3) + "  y " + nf(gravityY, 0, 3) + "  z " + nf(gravityZ, 0, 3), 40, 92);
  text("Keys: r reset", 40, height - 50);

  drawTiltDisk(width - 118, 72);

  if (won) {
    textAlign(CENTER, CENTER);
    textSize(46);
    fill(255, 205, 45);
    text("GOAL", width / 2, 46);
  }

  if (millis() - lastPacketMs > 2000) {
    fill(255, 190, 40);
    textAlign(LEFT, TOP);
    textSize(20);
    text("Enable BNO085 Gravity output in OpenDeck.", 40, 124);
  }
}

void drawTiltDisk(float x, float y) {
  PVector gravity = new PVector(gravityX, gravityY);
  float radius = 54;

  noFill();
  stroke(150);
  strokeWeight(2);
  ellipse(x, y, radius * 2, radius * 2);

  stroke(80);
  line(x - radius, y, x + radius, y);
  line(x, y - radius, x, y + radius);

  gravity.limit(1);

  float px = x + gravity.x * radius;
  float py = y - gravity.y * radius;

  noStroke();
  fill(255, 215, 45);
  ellipse(px, py, 16, 16);
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
  if (key == 'r' || key == 'R') {
    resetMarble();
  }
}
