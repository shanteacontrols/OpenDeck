/*
  OpenDeck VL53L5CX Raw Monitor

  Receives OpenDeck OSC rows:
    /opendeck/sensors/vl53l5cx/row/0  iiii or iiiiiiii
    ...

  This sketch intentionally does no filtering:
    - no interpolation
    - no fade
    - no invalid-zone hold
    - no debouncing

  Use it to inspect what the firmware is actually publishing.
  Left panel: raw distance.
  Right panel: raw frame-to-frame delta.
*/

import oscP5.*;
import netP5.*;

OscP5 oscP5;

int oscListenPort = 9000;
String rowPathPrefix = "/opendeck/sensors/vl53l5cx/row/";

int cols = 8;
int rows = 8;
int zoneCount = cols * rows;

int[] distances = new int[64];
int[] previousDistances = new int[64];
int[] deltas = new int[64];
int[] lastUpdateMs = new int[64];

int[] selectedHistory = new int[240];
int selectedHistoryWrite = 0;
int selectedZone = 0;

int frameCounter = 0;
int rowsSeenMask = 0;
int packetCounter = 0;
int lastPacketMs = 0;
int lastFrameMs = 0;
int previousFrameMs = 0;
boolean receivedData = false;

int nearMm = 50;
int farMm = 1800;
int deltaMaxMm = 250;
boolean showNumbers = true;

void setup() {
  size(1200, 950, P2D);
  surface.setTitle("OpenDeck VL53L5CX Raw Monitor");
  oscP5 = new OscP5(this, oscListenPort);

  textFont(createFont("SansSerif", 16));

  for (int i = 0; i < selectedHistory.length; i++) {
    selectedHistory[i] = 0;
  }
}

void draw() {
  background(8);
  drawDistancePanel();
  drawDeltaPanel();
  drawHistory();
  drawStatus();
}

void drawDistancePanel() {
  float margin = 38;
  float gap = 32;
  float statusHeight = 250;
  float panelSize = min((width - margin * 2 - gap) / 2.0, height - statusHeight - margin);
  float cell = panelSize / cols;
  float startX = margin;
  float startY = margin + 38;

  drawPanelTitle("Raw distance", startX, margin, panelSize);

  noStroke();

  for (int y = 0; y < rows; y++) {
    for (int x = 0; x < cols; x++) {
      int idx = x + y * cols;
      int distance = distances[idx];
      int age = millis() - lastUpdateMs[idx];
      boolean stale = receivedData && (age > 500);

      if (distance <= 0 || stale) {
        fill(stale ? 35 : 18);
      } else {
        float heat = constrain(map(distance, farMm, nearMm, 0.0, 1.0), 0.0, 1.0);
        colorMode(HSB, 360, 100, 100, 100);
        fill(220 - heat * 220, 90, 35 + heat * 60, 100);
        colorMode(RGB, 255);
      }

      rect(startX + x * cell + 4, startY + y * cell + 4, cell - 8, cell - 8, 8);

      if (idx == selectedZone) {
        drawSelectedCell(startX, startY, cell, x, y);
      }

      if (showNumbers) {
        fill(255);
        textAlign(CENTER, CENTER);
        textSize(15);
        text(distance, startX + x * cell + cell / 2.0, startY + y * cell + cell * 0.5);
      }
    }
  }
}

void drawDeltaPanel() {
  float margin = 38;
  float gap = 32;
  float statusHeight = 250;
  float panelSize = min((width - margin * 2 - gap) / 2.0, height - statusHeight - margin);
  float cell = panelSize / cols;
  float startX = margin + panelSize + gap;
  float startY = margin + 38;

  drawPanelTitle("Raw delta", startX, margin, panelSize);

  noStroke();

  for (int y = 0; y < rows; y++) {
    for (int x = 0; x < cols; x++) {
      int idx = x + y * cols;
      int delta = deltas[idx];
      float amount = constrain(abs(delta) / float(deltaMaxMm), 0.0, 1.0);

      if (delta == 0) {
        fill(18);
      } else if (delta > 0) {
        fill(255, 185 - 85 * amount, 20, 40 + 215 * amount);
      } else {
        fill(20, 150 + 80 * amount, 255, 40 + 215 * amount);
      }

      rect(startX + x * cell + 4, startY + y * cell + 4, cell - 8, cell - 8, 8);

      if (idx == selectedZone) {
        drawSelectedCell(startX, startY, cell, x, y);
      }

      if (showNumbers) {
        fill(255);
        textAlign(CENTER, CENTER);
        textSize(15);

        if (delta > 0) {
          text("+" + delta, startX + x * cell + cell / 2.0, startY + y * cell + cell * 0.5);
        } else {
          text(delta, startX + x * cell + cell / 2.0, startY + y * cell + cell * 0.5);
        }
      }
    }
  }
}

void drawPanelTitle(String title, float x, float y, float w) {
  fill(235);
  textAlign(CENTER, TOP);
  textSize(20);
  text(title, x + w / 2.0, y);
}

void drawSelectedCell(float startX, float startY, float cell, int x, int y) {
  noFill();
  stroke(255);
  strokeWeight(4);
  rect(startX + x * cell + 7, startY + y * cell + 7, cell - 14, cell - 14, 8);
  noStroke();
}

void drawGrid() {
  float margin = 44;
  float statusHeight = 250;
  float gridSize = min(width - margin * 2, height - statusHeight - margin);
  float cell = gridSize / cols;
  float startX = (width - gridSize) / 2.0;
  float startY = margin;

  noStroke();

  for (int y = 0; y < rows; y++) {
    for (int x = 0; x < cols; x++) {
      int idx = x + y * cols;
      int distance = distances[idx];
      int delta = deltas[idx];
      int age = millis() - lastUpdateMs[idx];
      boolean stale = receivedData && (age > 500);

      if (distance <= 0 || stale) {
        fill(stale ? 35 : 18);
      } else {
        float heat = constrain(map(distance, farMm, nearMm, 0.0, 1.0), 0.0, 1.0);
        colorMode(HSB, 360, 100, 100, 100);
        fill(220 - heat * 220, 90, 35 + heat * 60, 100);
        colorMode(RGB, 255);
      }

      rect(startX + x * cell + 4, startY + y * cell + 4, cell - 8, cell - 8, 8);

      if (idx == selectedZone) {
        noFill();
        stroke(255);
        strokeWeight(4);
        rect(startX + x * cell + 7, startY + y * cell + 7, cell - 14, cell - 14, 8);
        noStroke();
      }

      fill(255);
      textAlign(CENTER, CENTER);
      textSize(16);
      text(distance, startX + x * cell + cell / 2.0, startY + y * cell + cell * 0.43);

      textSize(12);
      if (delta > 0) {
        fill(255, 220, 120);
        text("+" + delta, startX + x * cell + cell / 2.0, startY + y * cell + cell * 0.65);
      } else if (delta < 0) {
        fill(130, 210, 255);
        text(delta, startX + x * cell + cell / 2.0, startY + y * cell + cell * 0.65);
      } else {
        fill(180);
        text("0", startX + x * cell + cell / 2.0, startY + y * cell + cell * 0.65);
      }
    }
  }
}

void drawHistory() {
  float left = 44;
  float top = height - 205;
  float w = width - 88;
  float h = 95;

  noFill();
  stroke(80);
  strokeWeight(1);
  rect(left, top, w, h);

  stroke(45);
  for (int i = 1; i < 5; i++) {
    float y = top + h * i / 5.0;
    line(left, y, left + w, y);
  }

  stroke(255, 190, 40);
  strokeWeight(2);
  noFill();
  beginShape();

  for (int i = 0; i < selectedHistory.length; i++) {
    int readIndex = (selectedHistoryWrite + i) % selectedHistory.length;
    int value = selectedHistory[readIndex];
    float x = map(i, 0, selectedHistory.length - 1, left, left + w);
    float y = value <= 0 ? top + h : map(constrain(value, nearMm, farMm), farMm, nearMm, top + h, top);
    vertex(x, y);
  }

  endShape();
}

void drawStatus() {
  int active = 0;
  int nearest = 0;
  int maxAbsDelta = 0;

  for (int i = 0; i < zoneCount; i++) {
    int distance = distances[i];

    if (distance > 0) {
      active++;

      if ((nearest == 0) || (distance < nearest)) {
        nearest = distance;
      }
    }

    maxAbsDelta = max(maxAbsDelta, abs(deltas[i]));
  }

  int selectedDistance = distances[selectedZone];
  int selectedDelta = deltas[selectedZone];
  int selectedX = selectedZone % cols;
  int selectedY = selectedZone / cols;
  int packetAge = receivedData ? millis() - lastPacketMs : 0;
  int framePeriod = (previousFrameMs == 0 || lastFrameMs == 0) ? 0 : lastFrameMs - previousFrameMs;

  fill(235);
  textAlign(LEFT, TOP);
  textSize(18);

  float y = height - 245;
  text("VL53L5CX raw monitor - no Processing-side filtering", 44, y);
  y += 28;
  text("Status: " + (receivedData ? "receiving OSC" : "waiting for OSC on port " + oscListenPort), 44, y);
  y += 28;
  text("Grid: " + cols + "x" + rows + "   Packets: " + packetCounter + "   Frames: " + frameCounter + "   Last packet age: " + packetAge + " ms", 44, y);
  y += 28;
  text("Active zones: " + active + " / " + zoneCount + "   Nearest: " + (nearest > 0 ? nearest + " mm" : "-") + "   Max abs delta: " + maxAbsDelta + " mm", 44, y);
  y += 28;
  text("Selected zone: " + selectedZone + "  x=" + selectedX + " y=" + selectedY + "   value=" + selectedDistance + " mm   delta=" + selectedDelta + " mm", 44, y);
  y += 28;
  text("Frame period: " + (framePeriod > 0 ? framePeriod + " ms" : "-") + "   Keys: [ / ] far range, - / + delta range, n numbers, click zone", 44, y);
}

void oscEvent(OscMessage message) {
  String address = message.addrPattern();

  if (!address.startsWith(rowPathPrefix)) {
    return;
  }

  int row = parseRow(address);

  if (row < 0) {
    return;
  }

  int valueCount = message.arguments().length;

  if (((valueCount != 4) || !message.checkTypetag("iiii")) &&
      ((valueCount != 8) || !message.checkTypetag("iiiiiiii"))) {
    return;
  }

  if (valueCount != cols) {
    setGridSize(valueCount);
  }

  if (row >= rows) {
    return;
  }

  packetCounter++;
  lastPacketMs = millis();
  receivedData = true;

  if (row == 0) {
    previousFrameMs = lastFrameMs;
    lastFrameMs = lastPacketMs;
    rowsSeenMask = 0;
  }

  rowsSeenMask |= 1 << row;

  if (rowsSeenMask == ((1 << rows) - 1)) {
    frameCounter++;
  }

  for (int x = 0; x < cols; x++) {
    int idx = x + row * cols;
    int distance = message.get(x).intValue();

    previousDistances[idx] = distances[idx];
    distances[idx] = distance;
    deltas[idx] = distances[idx] - previousDistances[idx];
    lastUpdateMs[idx] = lastPacketMs;

    if (idx == selectedZone) {
      selectedHistory[selectedHistoryWrite] = distance;
      selectedHistoryWrite = (selectedHistoryWrite + 1) % selectedHistory.length;
    }
  }
}

int parseRow(String address) {
  String suffix = address.substring(rowPathPrefix.length());

  try {
    return Integer.parseInt(suffix);
  } catch (Exception e) {
    return -1;
  }
}

void setGridSize(int width) {
  cols = width;
  rows = width;
  zoneCount = cols * rows;
  selectedZone = constrain(selectedZone, 0, zoneCount - 1);

  for (int i = 0; i < distances.length; i++) {
    distances[i] = 0;
    previousDistances[i] = 0;
    deltas[i] = 0;
    lastUpdateMs[i] = 0;
  }

  for (int i = 0; i < selectedHistory.length; i++) {
    selectedHistory[i] = 0;
  }

  selectedHistoryWrite = 0;
  rowsSeenMask = 0;
}

void mousePressed() {
  float margin = 38;
  float gap = 32;
  float statusHeight = 250;
  float panelSize = min((width - margin * 2 - gap) / 2.0, height - statusHeight - margin);
  float cell = panelSize / cols;
  float startX = mouseX > (margin + panelSize + gap / 2.0) ? margin + panelSize + gap : margin;
  float startY = margin + 38;

  int x = int((mouseX - startX) / cell);
  int y = int((mouseY - startY) / cell);

  if ((x < 0) || (x >= cols) || (y < 0) || (y >= rows)) {
    return;
  }

  selectedZone = x + y * cols;

  for (int i = 0; i < selectedHistory.length; i++) {
    selectedHistory[i] = distances[selectedZone];
  }
}

void keyPressed() {
  if (key == '[') {
    farMm = max(nearMm + 50, farMm - 100);
  } else if (key == ']') {
    farMm += 100;
  } else if (key == '-') {
    deltaMaxMm = max(10, deltaMaxMm - 25);
  } else if (key == '+' || key == '=') {
    deltaMaxMm += 25;
  } else if (key == 'n' || key == 'N') {
    showNumbers = !showNumbers;
  }
}
