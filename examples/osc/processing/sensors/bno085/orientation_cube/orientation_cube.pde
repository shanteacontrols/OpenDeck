/*
  OpenDeck BNO085 Orientation Cube

  Receives OpenDeck OSC IMU data:
    /opendeck/sensors/bno085/quaternion  ffff
    /opendeck/sensors/bno085/euler       fff

  Quaternion is used when available. Euler is used as a fallback.
*/

import oscP5.*;
import netP5.*;

OscP5 oscP5;
final Object orientationLock = new Object();

int oscListenPort = 9000;
String quaternionPath = "/opendeck/sensors/bno085/quaternion";
String eulerPath = "/opendeck/sensors/bno085/euler";

float qReal = 1.0;
float qI = 0.0;
float qJ = 0.0;
float qK = 0.0;

float zeroQReal = 1.0;
float zeroQI = 0.0;
float zeroQJ = 0.0;
float zeroQK = 0.0;

float yaw = 0.0;
float pitch = 0.0;
float roll = 0.0;

float zeroYaw = 0.0;
float zeroPitch = 0.0;
float zeroRoll = 0.0;

boolean useQuaternion = false;
boolean freezeView = false;
boolean showTrail = true;
boolean receivedData = false;
boolean hasQuaternionData = false;

int lastPacketMs = 0;
int lastQuaternionMs = 0;
int lastEulerMs = 0;

float viewRotX = -0.52;
float viewRotY = 0.0;
float viewRotZ = -0.72;
float zoom = 1.0;

float smoothYaw = 0.0;
float smoothPitch = 0.0;
float smoothRoll = 0.0;

float[] frameRelativeQ = new float[] { 1.0, 0.0, 0.0, 0.0 };

ArrayList<PVector> trail = new ArrayList<PVector>();
int maxTrailPoints = 140;

void setup() {
  size(1200, 900, P3D);
  surface.setTitle("OpenDeck BNO085 Orientation Cube");

  oscP5 = new OscP5(this, oscListenPort);
  textFont(createFont("SansSerif", 18));
}

void draw() {
  background(6);
  snapshotOrientation();
  updateTrailPoint();

  lights();
  ambientLight(70, 70, 75);
  directionalLight(210, 210, 220, -0.35, 0.55, -0.72);
  directionalLight(80, 110, 160, 0.55, -0.45, 0.3);

  pushMatrix();
  translate(width / 2, height / 2 + 50, -40);
  scale(zoom);
  rotateX(viewRotX);
  rotateZ(viewRotZ);
  rotateY(viewRotY);

  drawFloor();
  drawWorldAxes();
  drawOrientationTrail();
  drawCube();

  popMatrix();

  drawHud();
}

void snapshotOrientation() {
  float localQReal;
  float localQI;
  float localQJ;
  float localQK;
  float localZeroQReal;
  float localZeroQI;
  float localZeroQJ;
  float localZeroQK;
  float localYaw;
  float localPitch;
  float localRoll;
  float localZeroYaw;
  float localZeroPitch;
  float localZeroRoll;
  boolean localHasQuaternionData;
  int localLastQuaternionMs;
  int localLastEulerMs;

  synchronized (orientationLock) {
    localQReal = qReal;
    localQI = qI;
    localQJ = qJ;
    localQK = qK;
    localZeroQReal = zeroQReal;
    localZeroQI = zeroQI;
    localZeroQJ = zeroQJ;
    localZeroQK = zeroQK;
    localYaw = yaw;
    localPitch = pitch;
    localRoll = roll;
    localZeroYaw = zeroYaw;
    localZeroPitch = zeroPitch;
    localZeroRoll = zeroRoll;
    localHasQuaternionData = hasQuaternionData;
    localLastQuaternionMs = lastQuaternionMs;
    localLastEulerMs = lastEulerMs;
  }

  boolean quaternionFresh = localHasQuaternionData && (millis() - localLastQuaternionMs < 3000);
  boolean eulerFresh = millis() - localLastEulerMs < 1500;

  useQuaternion = localHasQuaternionData && (quaternionFresh || !eulerFresh);

  if (useQuaternion) {
    float[] current = normalizedQuaternion(localQReal, localQI, localQJ, localQK);
    float[] zeroInverse = inverseUnitQuaternion(normalizedQuaternion(localZeroQReal, localZeroQI, localZeroQJ, localZeroQK));
    frameRelativeQ = normalizedQuaternion(multiplyQuaternion(zeroInverse, current));

    float[] euler = quaternionToEuler(frameRelativeQ);
    smoothYaw = euler[0];
    smoothPitch = euler[1];
    smoothRoll = euler[2];
    return;
  }

  smoothYaw = wrapDegrees(localYaw - localZeroYaw);
  smoothPitch = wrapDegrees(localPitch - localZeroPitch);
  smoothRoll = wrapDegrees(localRoll - localZeroRoll);
}

void drawCube() {
  pushMatrix();

  if (useQuaternion) {
    applyQuaternionBasis(frameRelativeQ);
  } else {
    rotateZ(radians(smoothYaw));
    rotateX(radians(smoothPitch));
    rotateY(radians(smoothRoll));
  }

  drawBodyAxes();

  noStroke();
  specular(140);
  shininess(18);

  drawCubeFaces();

  noFill();
  stroke(255, 180);
  strokeWeight(2);
  box(280, 178, 138);

  popMatrix();
}

void drawCubeFaces() {
  float hx = 140.0;
  float hy = 89.0;
  float hz = 69.0;

  drawQuadFace(color(26, 125, 240),
               -hx, -hy, -hz,
                hx, -hy, -hz,
                hx,  hy, -hz,
               -hx,  hy, -hz);
  drawQuadFace(color(18, 72, 135),
               -hx,  hy,  hz,
                hx,  hy,  hz,
                hx, -hy,  hz,
               -hx, -hy,  hz);
  drawQuadFace(color(245, 155, 20),
               -hx, -hy,  hz,
                hx, -hy,  hz,
                hx, -hy, -hz,
               -hx, -hy, -hz);
  drawQuadFace(color(115, 60, 210),
               -hx,  hy, -hz,
                hx,  hy, -hz,
                hx,  hy,  hz,
               -hx,  hy,  hz);
  drawQuadFace(color(235, 65, 65),
                hx, -hy, -hz,
                hx, -hy,  hz,
                hx,  hy,  hz,
                hx,  hy, -hz);
  drawQuadFace(color(35, 180, 115),
               -hx,  hy, -hz,
               -hx,  hy,  hz,
               -hx, -hy,  hz,
               -hx, -hy, -hz);
}

void drawQuadFace(color c,
                  float x1, float y1, float z1,
                  float x2, float y2, float z2,
                  float x3, float y3, float z3,
                  float x4, float y4, float z4) {
  fill(c);
  beginShape(QUADS);
  vertex(x1, y1, z1);
  vertex(x2, y2, z2);
  vertex(x3, y3, z3);
  vertex(x4, y4, z4);
  endShape();
}

void drawBodyAxes() {
  strokeWeight(5);

  stroke(255, 70, 70);
  line(0, 0, 0, 210, 0, 0);

  stroke(70, 220, 130);
  line(0, 0, 0, 0, -210, 0);

  stroke(80, 150, 255);
  line(0, 0, 0, 0, 0, -210);
}

void drawWorldAxes() {
  strokeWeight(2);

  stroke(255, 60, 60, 130);
  line(-430, 0, 0, 430, 0, 0);

  stroke(70, 220, 130, 130);
  line(0, -310, 0, 0, 310, 0);

  stroke(70, 140, 255, 130);
  line(0, 0, -260, 0, 0, 260);
}

void drawFloor() {
  stroke(48);
  strokeWeight(1);
  noFill();

  int grid = 8;
  float spacing = 60;
  float half = grid * spacing / 2.0;

  for (int i = 0; i <= grid; i++) {
    float p = -half + i * spacing;
    line(-half, p, 150, half, p, 150);
    line(p, -half, 150, p, half, 150);
  }
}

void drawOrientationTrail() {
  if (!showTrail || trail.size() < 2) {
    return;
  }

  noFill();
  strokeWeight(3);

  for (int i = 1; i < trail.size(); i++) {
    float alpha = map(i, 1, trail.size() - 1, 20, 180);
    stroke(255, 180, 40, alpha);
    PVector a = trail.get(i - 1);
    PVector b = trail.get(i);
    line(a.x, a.y, a.z, b.x, b.y, b.z);
  }
}

void updateTrailPoint() {
  if (!showTrail || !receivedData) {
    return;
  }

  PVector point = useQuaternion
                ? rotateQuaternionPoint(new PVector(0, -220, -120), frameRelativeQ)
                : rotateEulerPoint(new PVector(0, -220, -120), smoothYaw, smoothPitch, smoothRoll);
  trail.add(point);

  while (trail.size() > maxTrailPoints) {
    trail.remove(0);
  }
}

void drawHud() {
  hint(DISABLE_DEPTH_TEST);
  camera();
  noLights();

  fill(235);
  textAlign(LEFT, TOP);
  textSize(20);

  String status = receivedData ? "receiving OSC" : "waiting for OSC on port " + oscListenPort;
  String mode = useQuaternion ? "quaternion" : "euler";

  text("BNO085 orientation cube", 38, 34);
  text("Status: " + status, 38, 66);
  text("Mode: " + mode, 38, 98);
  text("Yaw: " + nf(wrapDegrees(smoothYaw), 0, 1) + "  Pitch: " + nf(wrapDegrees(smoothPitch), 0, 1) + "  Roll: " + nf(wrapDegrees(smoothRoll), 0, 1), 38, 130);
  text("Keys: z zero, t trail, r reset view, +/- zoom", 38, height - 52);

  if (millis() - lastPacketMs > 2000) {
    fill(255, 190, 40);
    text("Enable BNO085 Quaternion or Euler output in OpenDeck.", 38, 164);
  }

  hint(ENABLE_DEPTH_TEST);
}

void oscEvent(OscMessage message) {
  String address = message.addrPattern();

  if (address.equals(quaternionPath) && message.checkTypetag("ffff")) {
    float[] next = normalizedQuaternion(message.get(0).floatValue(),
                                        message.get(1).floatValue(),
                                        message.get(2).floatValue(),
                                        message.get(3).floatValue());

    synchronized (orientationLock) {
      if (hasQuaternionData && (quaternionDot(new float[] { qReal, qI, qJ, qK }, next) < 0.0)) {
        next[0] = -next[0];
        next[1] = -next[1];
        next[2] = -next[2];
        next[3] = -next[3];
      }

      qReal = next[0];
      qI = next[1];
      qJ = next[2];
      qK = next[3];

      hasQuaternionData = true;
      lastQuaternionMs = millis();
      markReceived();
    }
  } else if (address.equals(eulerPath) && message.checkTypetag("fff")) {
    synchronized (orientationLock) {
      yaw = message.get(0).floatValue();
      pitch = message.get(1).floatValue();
      roll = message.get(2).floatValue();

      lastEulerMs = millis();
      markReceived();
    }
  }
}

void markReceived() {
  receivedData = true;
  lastPacketMs = millis();
}

void keyPressed() {
  if (key == 'z' || key == 'Z') {
    synchronized (orientationLock) {
      if (useQuaternion) {
        float[] zero = normalizedQuaternion(qReal, qI, qJ, qK);
        zeroQReal = zero[0];
        zeroQI = zero[1];
        zeroQJ = zero[2];
        zeroQK = zero[3];
      } else {
        zeroYaw = yaw;
        zeroPitch = pitch;
        zeroRoll = roll;
      }
    }

    trail.clear();
  } else if (key == 't' || key == 'T') {
    showTrail = !showTrail;
    trail.clear();
  } else if (key == 'r' || key == 'R') {
    viewRotX = -0.52;
    viewRotY = 0.0;
    viewRotZ = -0.72;
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

float[] normalizedQuaternion(float real, float i, float j, float k) {
  float norm = real * real + i * i + j * j + k * k;

  if (norm <= 0.0) {
    return new float[] { 1.0, 0.0, 0.0, 0.0 };
  }

  float invNorm = 1.0 / sqrt(norm);

  return new float[] { real * invNorm, i * invNorm, j * invNorm, k * invNorm };
}

float[] normalizedQuaternion(float[] q) {
  return normalizedQuaternion(q[0], q[1], q[2], q[3]);
}

float[] inverseUnitQuaternion(float[] q) {
  return new float[] { q[0], -q[1], -q[2], -q[3] };
}

float quaternionDot(float[] a, float[] b) {
  return (a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]) + (a[3] * b[3]);
}

float[] multiplyQuaternion(float[] a, float[] b) {
  float real = (a[0] * b[0]) - (a[1] * b[1]) - (a[2] * b[2]) - (a[3] * b[3]);
  float i = (a[0] * b[1]) + (a[1] * b[0]) + (a[2] * b[3]) - (a[3] * b[2]);
  float j = (a[0] * b[2]) - (a[1] * b[3]) + (a[2] * b[0]) + (a[3] * b[1]);
  float k = (a[0] * b[3]) + (a[1] * b[2]) - (a[2] * b[1]) + (a[3] * b[0]);

  return new float[] { real, i, j, k };
}

float[] quaternionToEuler(float[] q) {
  float real = q[0];
  float i = q[1];
  float j = q[2];
  float k = q[3];

  float yawValue = atan2(2.0 * ((i * j) + (k * real)),
                         i * i - j * j - k * k + real * real);
  float pitchValue = asin(constrain(-2.0 * ((i * k) - (j * real)), -1.0, 1.0));
  float rollValue = atan2(2.0 * ((j * k) + (i * real)),
                          -i * i - j * j + k * k + real * real);

  return new float[] { degrees(yawValue), degrees(pitchValue), degrees(rollValue) };
}

void applyQuaternionBasis(float[] q) {
  PVector xAxis = rotateQuaternionPoint(new PVector(1, 0, 0), q);
  PVector yAxis = rotateQuaternionPoint(new PVector(0, 1, 0), q);
  PVector zAxis = rotateQuaternionPoint(new PVector(0, 0, 1), q);

  applyMatrix(xAxis.x, yAxis.x, zAxis.x, 0.0,
              xAxis.y, yAxis.y, zAxis.y, 0.0,
              xAxis.z, yAxis.z, zAxis.z, 0.0,
              0.0,     0.0,     0.0,     1.0);
}

PVector rotateQuaternionPoint(PVector point, float[] q) {
  float real = q[0];
  float i = q[1];
  float j = q[2];
  float k = q[3];

  PVector vector = new PVector(point.x, point.y, point.z);
  PVector axis = new PVector(i, j, k);
  PVector cross = axis.cross(vector);
  PVector nestedCross = axis.cross(cross);

  cross.mult(2.0 * real);
  nestedCross.mult(2.0);
  vector.add(cross);
  vector.add(nestedCross);

  return vector;
}

PVector rotateEulerPoint(PVector point, float yawDegrees, float pitchDegrees, float rollDegrees) {
  float yawRadians = radians(yawDegrees);
  float pitchRadians = radians(pitchDegrees);
  float rollRadians = radians(rollDegrees);

  float x = point.x;
  float y = point.y;
  float z = point.z;

  float cosYaw = cos(yawRadians);
  float sinYaw = sin(yawRadians);
  float yawX = (x * cosYaw) - (y * sinYaw);
  float yawY = (x * sinYaw) + (y * cosYaw);
  x = yawX;
  y = yawY;

  float cosPitch = cos(pitchRadians);
  float sinPitch = sin(pitchRadians);
  float pitchY = (y * cosPitch) - (z * sinPitch);
  float pitchZ = (y * sinPitch) + (z * cosPitch);
  y = pitchY;
  z = pitchZ;

  float cosRoll = cos(rollRadians);
  float sinRoll = sin(rollRadians);
  float rollX = (x * cosRoll) + (z * sinRoll);
  float rollZ = (-x * sinRoll) + (z * cosRoll);

  return new PVector(rollX, y, rollZ);
}

float wrapDegrees(float value) {
  while (value > 180.0) {
    value -= 360.0;
  }

  while (value < -180.0) {
    value += 360.0;
  }

  return value;
}
