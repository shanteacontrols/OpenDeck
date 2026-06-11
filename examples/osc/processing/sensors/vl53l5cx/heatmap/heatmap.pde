/*
  OpenDeck VL53L5CX Heatmap

  Receives OpenDeck OSC rows:
    /opendeck/sensors/vl53l5cx/row/0  iiiiiiii
    ...
    /opendeck/sensors/vl53l5cx/row/7  iiiiiiii

  Each row carries eight distance values in millimeters. Invalid zones are 0.
*/

import oscP5.*;
import netP5.*;

OscP5 oscP5;

int oscListenPort = 9000;
String rowPathPrefix = "/opendeck/sensors/vl53l5cx/row/";

int cols = 8;
int rows = 8;
int zoneCount = cols * rows;

int[] depths = new int[zoneCount];
float[] visualDepths = new float[zoneCount];
int[] missingFrames = new int[zoneCount];

int invalidHoldFrames = 8;
float validSmoothing = 0.35;
float invalidFade = 0.92;

int nearMm = 120;
int farMm = 1800;
int minUsableMm = 50;
float blobThreshold = 0.12;

boolean receivedData = false;
int activeZones = 0;
int nearZones = 0;
int nearestDistance = 0;
int nearestZone = -1;
float blobX = 0.0;
float blobY = 0.0;
float blobDiameterCells = 0.0;

boolean usableDistance(float distance) {
  return (distance >= minUsableMm) && (distance <= farMm);
}

void setup() {
  size(1100, 1000, P2D);
  surface.setTitle("OpenDeck VL53L5CX Heatmap");
  oscP5 = new OscP5(this, oscListenPort);

  textFont(createFont("SansSerif", 18));

  for (int idx = 0; idx < zoneCount; idx++) {
    depths[idx] = 0;
    visualDepths[idx] = 0;
    missingFrames[idx] = invalidHoldFrames + 1;
  }
}

void draw() {
  background(8);
  updateVisualState();
  drawGrid();
  drawBlob();
  drawStatus();
}

void updateVisualState() {
  activeZones = 0;
  nearZones = 0;
  nearestDistance = 0;
  nearestZone = -1;

  float weightedX = 0.0;
  float weightedY = 0.0;
  float totalWeight = 0.0;

  for (int idx = 0; idx < zoneCount; idx++) {
    if (depths[idx] > 0) {
      visualDepths[idx] = lerp(visualDepths[idx], depths[idx], validSmoothing);
    } else if (missingFrames[idx] > invalidHoldFrames) {
      visualDepths[idx] *= invalidFade;
    }

    int distance = int(visualDepths[idx]);

    if (!usableDistance(distance)) {
      continue;
    }

    activeZones++;

    if ((nearestDistance == 0) || (distance < nearestDistance)) {
      nearestDistance = distance;
      nearestZone = idx;
    }

    float closeness = constrain(map(distance, farMm, nearMm, 0.0, 1.0), 0.0, 1.0);

    if (closeness <= blobThreshold) {
      continue;
    }

    int x = idx % cols;
    int y = idx / cols;
    float weight = closeness * closeness;

    nearZones++;
    weightedX += x * weight;
    weightedY += y * weight;
    totalWeight += weight;
  }

  if (totalWeight > 0.0) {
    float centerX = weightedX / totalWeight;
    float centerY = weightedY / totalWeight;
    float targetDiameter = constrain(0.7 + sqrt(nearZones) * 0.28, 0.9, 3.2);

    blobX = lerp(blobX, centerX, 0.25);
    blobY = lerp(blobY, centerY, 0.25);
    blobDiameterCells = lerp(blobDiameterCells, targetDiameter, 0.25);
  } else {
    blobDiameterCells = lerp(blobDiameterCells, 0.0, 0.2);
  }
}

void drawGrid() {
  float margin = 52;
  float gridSize = min(width - margin * 2, height - 180 - margin);
  float cell = gridSize / cols;
  float startX = (width - gridSize) / 2;
  float startY = margin;

  noStroke();

  for (int y = 0; y < rows; y++) {
    for (int x = 0; x < cols; x++) {
      int idx = x + y * cols;
      float distance = visualDepths[idx];
      float heat = constrain(map(distance, farMm, nearMm, 0.0, 1.0), 0.0, 1.0);

      if (!usableDistance(distance)) {
        fill(18);
      } else {
        colorMode(HSB, 360, 100, 100, 100);
        fill(220 - heat * 220, 90, 30 + heat * 70, 95);
        colorMode(RGB, 255);
      }

      rect(startX + x * cell + 3, startY + y * cell + 3, cell - 6, cell - 6, 12);

      if (usableDistance(distance)) {
        fill(255, 220);
        textAlign(CENTER, CENTER);
        textSize(13);
        text(int(distance), startX + x * cell + cell / 2, startY + y * cell + cell / 2);
      }
    }
  }

  if (nearestZone >= 0) {
    int nearestX = nearestZone % cols;
    int nearestY = nearestZone / cols;

    noFill();
    stroke(255);
    strokeWeight(3);
    rect(startX + nearestX * cell + 5, startY + nearestY * cell + 5, cell - 10, cell - 10, 10);
  }
}

void drawBlob() {
  if (nearZones == 0 || blobDiameterCells < 0.1) {
    return;
  }

  float margin = 52;
  float gridSize = min(width - margin * 2, height - 180 - margin);
  float cell = gridSize / cols;
  float startX = (width - gridSize) / 2;
  float startY = margin;

  float x = startX + (blobX + 0.5) * cell;
  float y = startY + (blobY + 0.5) * cell;
  float diameter = blobDiameterCells * cell;

  noFill();
  stroke(255, 255, 255, 210);
  strokeWeight(4);
  ellipse(x, y, diameter, diameter);

  fill(255);
  noStroke();
  ellipse(x, y, 10, 10);
}

void drawStatus() {
  fill(235);
  textAlign(LEFT, TOP);
  textSize(18);

  String nearestText = nearestDistance > 0 ? nearestDistance + " mm" : "-";
  String dataText = receivedData ? "receiving OSC" : "waiting for OSC on port " + oscListenPort;

  text("VL53L5CX heatmap", 52, height - 118);
  text("Status: " + dataText, 52, height - 88);
  text("Active zones: " + activeZones + " / " + zoneCount + "   Near zones: " + nearZones, 52, height - 58);
  text("Nearest: " + nearestText, 52, height - 28);
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
