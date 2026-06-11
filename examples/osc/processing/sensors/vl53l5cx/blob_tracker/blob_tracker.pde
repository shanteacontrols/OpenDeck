/*
  OpenDeck VL53L5CX Blob Tracker

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
boolean[] foreground = new boolean[zoneCount];
boolean[] visited = new boolean[zoneCount];
int[] queue = new int[zoneCount];

int invalidHoldFrames = 12;
float validSmoothing = 0.25;
float invalidFade = 0.94;

int minUsableMm = 50;
int foregroundCutoffMm = 1000;
int foregroundHysteresisMm = 150;
int minBlobZones = 2;

boolean receivedData = false;

int activeZones = 0;
int foregroundZones = 0;
int blobZones = 0;
int blobNearestMm = 0;
int blobMinX = 0;
int blobMinY = 0;
int blobMaxX = 0;
int blobMaxY = 0;
float blobX = 0.0;
float blobY = 0.0;
float blobSize = 0.0;
float smoothBlobX = 0.0;
float smoothBlobY = 0.0;
float smoothBlobSize = 0.0;
boolean blobPresent = false;

boolean usableDistance(float distance) {
  return (distance >= minUsableMm);
}

void setup() {
  size(1100, 1000, P2D);
  surface.setTitle("OpenDeck VL53L5CX Blob Tracker");
  oscP5 = new OscP5(this, oscListenPort);

  textFont(createFont("SansSerif", 18));

  for (int idx = 0; idx < zoneCount; idx++) {
    depths[idx] = 0;
    visualDepths[idx] = 0.0;
    missingFrames[idx] = invalidHoldFrames + 1;
    foreground[idx] = false;
    visited[idx] = false;
  }
}

void draw() {
  background(8);
  updateVisualState();
  findLargestBlob();
  drawGrid();
  drawBlobOverlay();
  drawStatus();
}

void updateVisualState() {
  activeZones = 0;
  foregroundZones = 0;

  for (int idx = 0; idx < zoneCount; idx++) {
    if (depths[idx] > 0) {
      visualDepths[idx] = lerp(visualDepths[idx], depths[idx], validSmoothing);
    } else if (missingFrames[idx] > invalidHoldFrames) {
      visualDepths[idx] *= invalidFade;
    }

    float distance = visualDepths[idx];

    if (usableDistance(distance)) {
      activeZones++;
    }

    int cutoff = foreground[idx] ? foregroundCutoffMm + foregroundHysteresisMm : foregroundCutoffMm;
    foreground[idx] = usableDistance(distance) && (distance <= cutoff);

    if (foreground[idx]) {
      foregroundZones++;
    }

    visited[idx] = false;
  }
}

void findLargestBlob() {
  int bestCount = 0;
  int bestNearest = 0;
  int bestMinX = 0;
  int bestMinY = 0;
  int bestMaxX = 0;
  int bestMaxY = 0;
  float bestWeightedX = 0.0;
  float bestWeightedY = 0.0;
  float bestWeight = 0.0;

  for (int idx = 0; idx < zoneCount; idx++) {
    if (!foreground[idx] || visited[idx]) {
      continue;
    }

    int head = 0;
    int tail = 0;
    int count = 0;
    int nearest = 0;
    int minX = cols - 1;
    int minY = rows - 1;
    int maxX = 0;
    int maxY = 0;
    float weightedX = 0.0;
    float weightedY = 0.0;
    float totalWeight = 0.0;

    queue[tail++] = idx;
    visited[idx] = true;

    while (head < tail) {
      int current = queue[head++];
      int x = current % cols;
      int y = current / cols;
      int distance = int(visualDepths[current]);
      float weight = constrain(map(distance, foregroundCutoffMm, minUsableMm, 0.1, 1.0), 0.1, 1.0);

      count++;
      minX = min(minX, x);
      minY = min(minY, y);
      maxX = max(maxX, x);
      maxY = max(maxY, y);
      weightedX += x * weight;
      weightedY += y * weight;
      totalWeight += weight;

      if ((nearest == 0) || (distance < nearest)) {
        nearest = distance;
      }

      for (int yy = max(0, y - 1); yy <= min(rows - 1, y + 1); yy++) {
        for (int xx = max(0, x - 1); xx <= min(cols - 1, x + 1); xx++) {
          int neighbor = xx + yy * cols;

          if (visited[neighbor] || !foreground[neighbor]) {
            continue;
          }

          visited[neighbor] = true;
          queue[tail++] = neighbor;
        }
      }
    }

    if (count > bestCount) {
      bestCount = count;
      bestNearest = nearest;
      bestMinX = minX;
      bestMinY = minY;
      bestMaxX = maxX;
      bestMaxY = maxY;
      bestWeightedX = weightedX;
      bestWeightedY = weightedY;
      bestWeight = totalWeight;
    }
  }

  blobPresent = bestCount >= minBlobZones;
  blobZones = blobPresent ? bestCount : 0;
  blobNearestMm = blobPresent ? bestNearest : 0;

  if (blobPresent && (bestWeight > 0.0)) {
    blobX = bestWeightedX / bestWeight;
    blobY = bestWeightedY / bestWeight;
    blobSize = float(bestCount) / float(zoneCount);
    blobMinX = bestMinX;
    blobMinY = bestMinY;
    blobMaxX = bestMaxX;
    blobMaxY = bestMaxY;

    smoothBlobX = lerp(smoothBlobX, blobX, 0.25);
    smoothBlobY = lerp(smoothBlobY, blobY, 0.25);
    smoothBlobSize = lerp(smoothBlobSize, blobSize, 0.25);
  } else {
    smoothBlobSize = lerp(smoothBlobSize, 0.0, 0.18);
  }
}

void drawGrid() {
  float margin = 64;
  float gridSize = min(width - margin * 2, height - 220 - margin);
  float cell = gridSize / cols;
  float startX = (width - gridSize) / 2;
  float startY = margin;

  noStroke();

  for (int y = 0; y < rows; y++) {
    for (int x = 0; x < cols; x++) {
      int idx = x + y * cols;
      float distance = visualDepths[idx];

      if (!usableDistance(distance)) {
        fill(18);
      } else if (foreground[idx]) {
        float heat = constrain(map(distance, foregroundCutoffMm, minUsableMm, 0.0, 1.0), 0.0, 1.0);
        colorMode(HSB, 360, 100, 100, 100);
        fill(45 - heat * 45, 90, 45 + heat * 55, 96);
        colorMode(RGB, 255);
      } else {
        float cool = constrain(map(distance, foregroundCutoffMm, 2200, 0.0, 1.0), 0.0, 1.0);
        fill(20, 80 + cool * 60, 115 + cool * 70);
      }

      rect(startX + x * cell + 4, startY + y * cell + 4, cell - 8, cell - 8, 12);

      if (usableDistance(distance)) {
        fill(255, foreground[idx] ? 235 : 165);
        textAlign(CENTER, CENTER);
        textSize(13);
        text(int(distance), startX + x * cell + cell / 2, startY + y * cell + cell / 2);
      }
    }
  }
}

void drawBlobOverlay() {
  if (!blobPresent && smoothBlobSize < 0.01) {
    return;
  }

  float margin = 64;
  float gridSize = min(width - margin * 2, height - 220 - margin);
  float cell = gridSize / cols;
  float startX = (width - gridSize) / 2;
  float startY = margin;

  if (blobPresent) {
    noFill();
    stroke(255);
    strokeWeight(4);
    rect(startX + blobMinX * cell + 6,
         startY + blobMinY * cell + 6,
         ((blobMaxX - blobMinX) + 1) * cell - 12,
         ((blobMaxY - blobMinY) + 1) * cell - 12,
         12);
  }

  float centerX = startX + (smoothBlobX + 0.5) * cell;
  float centerY = startY + (smoothBlobY + 0.5) * cell;
  float diameter = max(18, smoothBlobSize * gridSize * 0.75);

  noFill();
  stroke(255, 220);
  strokeWeight(3);
  ellipse(centerX, centerY, diameter, diameter);

  fill(255);
  noStroke();
  ellipse(centerX, centerY, 12, 12);
}

void drawStatus() {
  fill(235);
  textAlign(LEFT, TOP);
  textSize(18);

  String dataText = receivedData ? "receiving OSC" : "waiting for OSC on port " + oscListenPort;
  String blobText = blobPresent ? "present" : "none";
  float normX = blobPresent ? blobX / float(cols - 1) : 0.0;
  float normY = blobPresent ? blobY / float(rows - 1) : 0.0;

  text("VL53L5CX blob tracker", 64, height - 174);
  text("Status: " + dataText, 64, height - 144);
  text("Active zones: " + activeZones + " / " + zoneCount + "   Foreground zones: " + foregroundZones, 64, height - 114);
  text("Blob: " + blobText + "   zones: " + blobZones + "   nearest: " + (blobNearestMm > 0 ? blobNearestMm + " mm" : "-"), 64, height - 84);
  text("Center: x=" + nf(normX, 1, 2) + " y=" + nf(normY, 1, 2) + "   size=" + nf(blobSize, 1, 2), 64, height - 54);
  text("Keys: [ ] cutoff (" + foregroundCutoffMm + " mm), - + min blob zones (" + minBlobZones + ")", 64, height - 24);
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

void keyPressed() {
  if (key == '[') {
    foregroundCutoffMm = max(minUsableMm, foregroundCutoffMm - 100);
  } else if (key == ']') {
    foregroundCutoffMm += 100;
  } else if (key == '-') {
    minBlobZones = max(1, minBlobZones - 1);
  } else if ((key == '=') || (key == '+')) {
    minBlobZones = min(zoneCount, minBlobZones + 1);
  }
}
