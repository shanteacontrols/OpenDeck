/*
  OpenDeck VL53L5CX Point Cloud

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
int[] pointConfidence = new int[zoneCount];
boolean[] pointCandidate = new boolean[zoneCount];
boolean[] pointVisible = new boolean[zoneCount];

int invalidHoldFrames = 18;
float validSmoothing = 0.18;
float invalidFade = 0.97;

int minUsableMm = 50;
int nearMm = 120;
int farMm = 1800;
int foregroundCutoffMm = 1000;
int foregroundHysteresisMm = 150;
int pointAppearFrames = 3;
int pointHoldFrames = 12;
int minNeighborSupport = 1;
int neighborDistanceMm = 450;
boolean foregroundOnly = false;

boolean receivedData = false;
int activeZones = 0;
int nearestDistance = 0;

float xPress = 0.0;
float yPress = 0.0;
float xRotOffset = 0.0;
float xRotPos = 0.0;
float zRotOffset = 0.0;
float zRotPos = 0.0;
float zoom = 1.0;

boolean usableDistance(float distance) {
  return (distance >= minUsableMm) && (distance <= farMm);
}

void setup() {
  size(1100, 900, P3D);
  surface.setTitle("OpenDeck VL53L5CX Point Cloud");
  oscP5 = new OscP5(this, oscListenPort);

  textFont(createFont("SansSerif", 18));

  for (int idx = 0; idx < zoneCount; idx++) {
    depths[idx] = 0;
    visualDepths[idx] = 0.0;
    missingFrames[idx] = invalidHoldFrames + 1;
    pointConfidence[idx] = 0;
    pointCandidate[idx] = false;
    pointVisible[idx] = false;
  }
}

void draw() {
  background(5);
  updateVisualState();

  lights();
  directionalLight(160, 160, 160, -0.4, 0.7, -0.6);

  pushMatrix();
  translate(width / 2, height / 2 + 40, -180);
  rotateX(PI / 3 - (xRotOffset + xRotPos));
  rotateZ(0 - zRotOffset - zRotPos);
  scale(zoom);

  drawReferenceGrid();
  drawPoints();

  popMatrix();

  drawStatus();
}

void updateVisualState() {
  activeZones = 0;
  nearestDistance = 0;

  for (int idx = 0; idx < zoneCount; idx++) {
    if (depths[idx] > 0) {
      visualDepths[idx] = lerp(visualDepths[idx], depths[idx], validSmoothing);
    } else if (missingFrames[idx] > invalidHoldFrames) {
      visualDepths[idx] *= invalidFade;
    }

    pointCandidate[idx] = candidateDistance(idx, depths[idx]);
  }

  for (int idx = 0; idx < zoneCount; idx++) {
    updatePointVisibility(idx);

    int distance = int(visualDepths[idx]);

    if (!pointVisible[idx] || !usableDistance(distance)) {
      continue;
    }

    activeZones++;

    if ((nearestDistance == 0) || (distance < nearestDistance)) {
      nearestDistance = distance;
    }
  }
}

void drawReferenceGrid() {
  float spacing = 80;
  float halfW = (cols - 1) * spacing / 2.0;
  float halfH = (rows - 1) * spacing / 2.0;

  stroke(50);
  strokeWeight(1);

  for (int x = 0; x < cols; x++) {
    line(x * spacing - halfW, -halfH, 0, x * spacing - halfW, halfH, 0);
  }

  for (int y = 0; y < rows; y++) {
    line(-halfW, y * spacing - halfH, 0, halfW, y * spacing - halfH, 0);
  }
}

void drawPoints() {
  float spacing = 80;
  float halfW = (cols - 1) * spacing / 2.0;
  float halfH = (rows - 1) * spacing / 2.0;

  noStroke();

  for (int y = 0; y < rows; y++) {
    for (int x = 0; x < cols; x++) {
      int idx = x + y * cols;
      float distance = visualDepths[idx];

      if (!pointVisible[idx] || !usableDistance(distance)) {
        continue;
      }

      float heat = constrain(map(distance, farMm, nearMm, 0.0, 1.0), 0.0, 1.0);
      float px = x * spacing - halfW;
      float py = y * spacing - halfH;
      float pz = -map(distance, nearMm, farMm, 220, -160);
      float radius = 10 + heat * 18;

      colorMode(HSB, 360, 100, 100, 100);
      fill(220 - heat * 220, 90, 35 + heat * 65, 95);
      colorMode(RGB, 255);

      pushMatrix();
      translate(px, py, pz);
      sphereDetail(12);
      sphere(radius);
      popMatrix();

      stroke(255, 45);
      strokeWeight(1);
      line(px, py, 0, px, py, pz);
      noStroke();
    }
  }
}

void updatePointVisibility(int idx) {
  boolean candidate = pointCandidate[idx] && hasNeighborSupport(idx);

  if (candidate) {
    pointConfidence[idx] = min(pointHoldFrames, pointConfidence[idx] + 1);
  } else {
    pointConfidence[idx] = max(0, pointConfidence[idx] - 1);
  }

  pointVisible[idx] = (pointConfidence[idx] >= pointAppearFrames) ||
                      (pointVisible[idx] && (pointConfidence[idx] > 0));
}

boolean candidateDistance(int idx, int distance) {
  if (!usableDistance(distance)) {
    return false;
  }

  if (!foregroundOnly) {
    return true;
  }

  int cutoff = pointVisible[idx] ? foregroundCutoffMm + foregroundHysteresisMm : foregroundCutoffMm;

  return distance <= cutoff;
}

boolean hasNeighborSupport(int idx) {
  if (!pointCandidate[idx]) {
    return false;
  }

  int x = idx % cols;
  int y = idx / cols;
  int support = 0;

  for (int yy = max(0, y - 1); yy <= min(rows - 1, y + 1); yy++) {
    for (int xx = max(0, x - 1); xx <= min(cols - 1, x + 1); xx++) {
      int neighbor = xx + yy * cols;

      if (neighbor == idx || !pointCandidate[neighbor]) {
        continue;
      }

      if (abs(depths[neighbor] - depths[idx]) <= neighborDistanceMm) {
        support++;
      }
    }
  }

  return support >= minNeighborSupport;
}

void drawStatus() {
  hint(DISABLE_DEPTH_TEST);
  camera();
  noLights();

  fill(235);
  textAlign(LEFT, TOP);
  textSize(18);

  String nearestText = nearestDistance > 0 ? nearestDistance + " mm" : "-";
  String dataText = receivedData ? "receiving OSC" : "waiting for OSC on port " + oscListenPort;

  text("VL53L5CX point cloud", 36, 34);
  text("Status: " + dataText, 36, 64);
  text("Active zones: " + activeZones + " / " + zoneCount, 36, 94);
  text("Nearest: " + nearestText, 36, 124);
  text("Mode: " + (foregroundOnly ? "foreground <= " + foregroundCutoffMm + " mm" : "full depth"), 36, 154);
  text("Debounce: appear " + pointAppearFrames + " frames, hold " + pointHoldFrames + " frames, neighbor " + minNeighborSupport, 36, 184);
  text("Keys: f foreground, [ ] cutoff, - + smooth, n/m neighbor", 36, 214);

  hint(ENABLE_DEPTH_TEST);
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

void mousePressed() {
  xPress = mouseX;
  yPress = mouseY;
}

void mouseDragged() {
  xRotOffset = (mouseY - yPress) / 100;
  zRotOffset = (mouseX - xPress) / 100;
}

void mouseReleased() {
  xRotPos += xRotOffset;
  xRotOffset = 0;
  zRotPos += zRotOffset;
  zRotOffset = 0;
}

void mouseWheel(MouseEvent event) {
  zoom = constrain(zoom - event.getCount() * 0.08, 0.35, 3.0);
}

void keyPressed() {
  if ((key == 'f') || (key == 'F')) {
    foregroundOnly = !foregroundOnly;
  } else if (key == '[') {
    foregroundCutoffMm = max(minUsableMm, foregroundCutoffMm - 100);
  } else if (key == ']') {
    foregroundCutoffMm = min(farMm, foregroundCutoffMm + 100);
  } else if (key == '-') {
    pointAppearFrames = min(10, pointAppearFrames + 1);
    pointHoldFrames = min(45, pointHoldFrames + 3);
  } else if ((key == '=') || (key == '+')) {
    pointAppearFrames = max(1, pointAppearFrames - 1);
    pointHoldFrames = max(3, pointHoldFrames - 3);
  } else if ((key == 'n') || (key == 'N')) {
    minNeighborSupport = min(3, minNeighborSupport + 1);
  } else if ((key == 'm') || (key == 'M')) {
    minNeighborSupport = max(0, minNeighborSupport - 1);
  }
}
