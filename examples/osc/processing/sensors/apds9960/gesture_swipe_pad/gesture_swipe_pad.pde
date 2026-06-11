/*
  OpenDeck APDS9960 Gesture Swipe Pad

  Receives OpenDeck OSC gesture data:
    /opendeck/sensors/apds9960/gesture  s
*/

import oscP5.*;
import netP5.*;

OscP5 oscP5;

int oscListenPort = 9000;
String gesturePath = "/opendeck/sensors/apds9960/gesture";

String lastGesture = "";
int lastPacketMs = 0;
int packetCount = 0;
boolean receivedData = false;

ArrayList<Card> cards = new ArrayList<Card>();
ArrayList<String> history = new ArrayList<String>();
ArrayList<Trail> trails = new ArrayList<Trail>();

Card outgoingCard = null;
String outgoingGesture = "";
float outgoingProgress = 1.0;

int[] palette;

void setup() {
  size(1200, 900, P2D);
  surface.setTitle("OpenDeck APDS9960 Card Swipe");

  oscP5 = new OscP5(this, oscListenPort);
  textFont(createFont("SansSerif", 18));

  palette = new int[] {
    color(36, 145, 212),
    color(250, 174, 42),
    color(116, 215, 96),
    color(233, 75, 132),
    color(115, 98, 235),
    color(34, 198, 181),
    color(240, 111, 55)
  };

  for (int i = 0; i < 7; i++) {
    cards.add(new Card(i, palette[i % palette.length]));
  }
}

void draw() {
  background(7, 8, 12);

  updateOutgoingCard();
  updateTrails();
  drawDeck();
  drawDirectionHints();
  drawHistory();
  drawHud();
}

void drawDeck() {
  float cx = width / 2.0;
  float cy = height / 2.0 + 38;

  for (int i = cards.size() - 1; i >= 0; i--) {
    Card card = cards.get(i);
    float depth = i;
    float x = cx + depth * 14;
    float y = cy + depth * 12;
    float scaleValue = 1.0 - depth * 0.035;
    float alpha = map(i, 0, cards.size() - 1, 255, 95);

    if (card == outgoingCard) {
      PVector offset = swipeOffset(outgoingGesture, easeOutCubic(outgoingProgress));
      x += offset.x;
      y += offset.y;
      scaleValue *= 1.0 + outgoingProgress * 0.06;
      alpha = 255 * (1.0 - outgoingProgress * 0.46);
    }

    card.draw(x, y, scaleValue, alpha, i == 0 && card != outgoingCard);
  }
}

void drawDirectionHints() {
  float cx = width / 2.0;
  float cy = height / 2.0 + 38;
  float cardW = cardWidth();
  float cardH = cardHeight();

  stroke(255, 40);
  strokeWeight(2);
  noFill();
  rect(cx - cardW / 2.0 - 22, cy - cardH / 2.0 - 22, cardW + 44, cardH + 44, 34);

  fill(255, 120);
  noStroke();
  textAlign(CENTER, CENTER);
  textSize(16);
  text("up", cx, cy - cardH / 2.0 - 58);
  text("down", cx, cy + cardH / 2.0 + 58);
  text("left", cx - cardW / 2.0 - 70, cy);
  text("right", cx + cardW / 2.0 + 74, cy);

  drawHintArrow(cx, cy - cardH / 2.0 - 32, "up");
  drawHintArrow(cx, cy + cardH / 2.0 + 32, "down");
  drawHintArrow(cx - cardW / 2.0 - 36, cy, "left");
  drawHintArrow(cx + cardW / 2.0 + 36, cy, "right");
}

void drawHintArrow(float x, float y, String gesture) {
  pushMatrix();
  translate(x, y);

  if (gesture.equals("right")) {
    rotate(HALF_PI);
  } else if (gesture.equals("down")) {
    rotate(PI);
  } else if (gesture.equals("left")) {
    rotate(-HALF_PI);
  }

  fill(255, 88);
  noStroke();
  triangle(0, -15, -10, 5, 10, 5);
  rect(-4, 4, 8, 13, 3);

  popMatrix();
}

void updateOutgoingCard() {
  if (outgoingCard == null) {
    return;
  }

  outgoingProgress += 0.046;

  if (outgoingProgress >= 1.0) {
    cards.remove(outgoingCard);
    cards.add(outgoingCard);
    outgoingCard = null;
    outgoingGesture = "";
    outgoingProgress = 1.0;
  }
}

void updateTrails() {
  for (int i = trails.size() - 1; i >= 0; i--) {
    Trail trail = trails.get(i);
    trail.update();
    trail.draw();

    if (trail.done()) {
      trails.remove(i);
    }
  }
}

void drawHistory() {
  float x = 40;
  float y = height - 102;

  fill(245);
  textAlign(LEFT, TOP);
  textSize(18);
  text("history", x, y - 34);

  for (int i = 0; i < history.size(); i++) {
    int alpha = int(map(i, 0, max(1, history.size() - 1), 230, 60));
    fill(255, alpha);
    text(history.get(i), x + i * 78, y);
  }
}

void drawHud() {
  fill(245);
  textAlign(LEFT, TOP);
  textSize(22);
  text("APDS9960 card swipe", 40, 36);

  textSize(18);
  text("Status: " + statusText(), 40, 72);
  text("Packets: " + packetCount, 40, 102);
  text("Last: " + (lastGesture.length() == 0 ? "-" : lastGesture), 40, 132);
  text("OSC: " + gesturePath, 40, 162);
}

String statusText() {
  if (!receivedData) {
    return "waiting for OSC";
  }

  if (millis() - lastPacketMs > 1800) {
    return "stale";
  }

  return "receiving OSC";
}

void oscEvent(OscMessage message) {
  if (!message.checkAddrPattern(gesturePath)) {
    return;
  }

  if (message.typetag().length() < 1 || message.typetag().charAt(0) != 's') {
    return;
  }

  String gesture = message.get(0).stringValue().toLowerCase();

  if (!validGesture(gesture)) {
    return;
  }

  handleGesture(gesture);
}

void handleGesture(String gesture) {
  lastGesture = gesture;
  receivedData = true;
  lastPacketMs = millis();
  packetCount++;

  if (outgoingCard == null && cards.size() > 0) {
    outgoingCard = cards.get(0);
    outgoingGesture = gesture;
    outgoingProgress = 0.0;
    trails.add(new Trail(gesture, outgoingCard.cardColor));
  }

  history.add(0, gesture);

  while (history.size() > 12) {
    history.remove(history.size() - 1);
  }
}

boolean validGesture(String gesture) {
  return gesture.equals("up") ||
         gesture.equals("down") ||
         gesture.equals("left") ||
         gesture.equals("right");
}

float cardWidth() {
  return min(width * 0.42, 430);
}

float cardHeight() {
  return min(height * 0.54, 520);
}

PVector swipeOffset(String gesture, float amount) {
  float distance = max(width, height) * 0.82;

  if (gesture.equals("up")) {
    return new PVector(0, -distance * amount);
  }

  if (gesture.equals("down")) {
    return new PVector(0, distance * amount);
  }

  if (gesture.equals("left")) {
    return new PVector(-distance * amount, 0);
  }

  return new PVector(distance * amount, 0);
}

float easeOutCubic(float x) {
  x = constrain(x, 0.0, 1.0);
  return 1.0 - pow(1.0 - x, 3.0);
}

class Card {
  int index;
  int cardColor;

  Card(int index, int cardColor) {
    this.index = index;
    this.cardColor = cardColor;
  }

  void draw(float x, float y, float scaleValue, float alpha, boolean topCard) {
    float w = cardWidth() * scaleValue;
    float h = cardHeight() * scaleValue;
    float left = x - w / 2.0;
    float top = y - h / 2.0;

    noStroke();
    fill(0, alpha * 0.28);
    rect(left + 18, top + 22, w, h, 28);

    fill(16, 19, 26, alpha);
    rect(left, top, w, h, 28);

    fill(red(cardColor), green(cardColor), blue(cardColor), alpha * 0.22);
    rect(left, top, w, h, 28);

    stroke(red(cardColor), green(cardColor), blue(cardColor), alpha * 0.85);
    strokeWeight(topCard ? 4 : 2);
    noFill();
    rect(left + 10, top + 10, w - 20, h - 20, 24);

    noStroke();
    fill(255, alpha * 0.94);
    textAlign(LEFT, TOP);
    textSize(34 * scaleValue);
    text("Card " + nf(index + 1, 2), left + 32 * scaleValue, top + 34 * scaleValue);

    fill(255, alpha * 0.54);
    textSize(17 * scaleValue);
    text("Swipe over the APDS9960", left + 34 * scaleValue, top + 84 * scaleValue);

    float iconX = x;
    float iconY = top + h * 0.62;
    fill(red(cardColor), green(cardColor), blue(cardColor), alpha * 0.92);
    ellipse(iconX, iconY, 94 * scaleValue, 94 * scaleValue);

    fill(255, alpha * 0.86);
    textAlign(CENTER, CENTER);
    textSize(44 * scaleValue);
    text(str(index + 1), iconX, iconY - 4 * scaleValue);
  }
}

class Trail {
  String gesture;
  int trailColor;
  float life = 1.0;

  Trail(String gesture, int trailColor) {
    this.gesture = gesture;
    this.trailColor = trailColor;
  }

  void update() {
    life -= 0.035;
  }

  void draw() {
    float cx = width / 2.0;
    float cy = height / 2.0 + 38;
    PVector end = swipeOffset(gesture, 0.22 + (1.0 - life) * 0.34);
    float alpha = constrain(life, 0.0, 1.0) * 180;

    stroke(red(trailColor), green(trailColor), blue(trailColor), alpha);
    strokeWeight(8 * life + 1);
    line(cx, cy, cx + end.x, cy + end.y);

    noStroke();
    fill(red(trailColor), green(trailColor), blue(trailColor), alpha);
    ellipse(cx + end.x, cy + end.y, 30 + (1.0 - life) * 42, 30 + (1.0 - life) * 42);
  }

  boolean done() {
    return life <= 0.0;
  }
}
