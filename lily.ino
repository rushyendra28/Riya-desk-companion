/*
  ───────────────────────────────────────────────────────────────────────
  Pan‑Tilt “Lily” Demo  •  Smooth Eye Blinking & Expressions  •  v1.0
  ───────────────────────────────────────────────────────────────────────
  ● On power‑up the OLED shows “THIS IS LILY” for 2 s.
  ● Continuous loop:
        – Base‑servo (pan) on pin 9 sweeps ±45 ° every 5 s.
        – At each end‑stop the head‑servo (tilt) nods (±10 °) while
          the eyes perform a short scripted routine: wake‑up ➜ centre ➜
          saccade right ➜ saccade left ➜ two blinks ➜ happy‑eyes.
  • All timing and geometry constants are grouped at the top.
  • Code is split into three sections:
        ①  Global constants / variables
        ②  OLED graphics helpers
        ③  Servo motion helpers
  ───────────────────────────────────────────────────────────────────────
*/

#include <Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ─── ① USER‑TWEAKABLE CONSTANTS ────────────────────────────────────────
const byte  BASE_PIN   = 9;      // pan
const byte  HEAD_PIN   = 10;     // tilt

const int   BASE_CENTRE = 90;    // neutral angles
const int   BASE_DELTA  = 30;    // ±30°
const int   HEAD_CENTRE = 90;
const int   HEAD_DELTA  = 30;    // ±30°

const unsigned long BASE_PAUSE = 1500;  // 1.5 s at each pan limit
const unsigned long HEAD_PAUSE = 400;   // pause between nod steps

// OLED geometry
#define OLED_WIDTH   128
#define OLED_HEIGHT   64
#define OLED_ADDR   0x3C
#define OLED_RESET   -1            // share MCU reset
// default eye reference
const int REF_EYE_W  = 40;
const int REF_EYE_H  = 40;
const int REF_SPACE  = 10;
const int REF_RAD    = 10;

// ─── ② GLOBAL OBJECTS & VARIABLES ──────────────────────────────────────
Servo baseServo, headServo;
Adafruit_SSD1306 oled(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);

// eye state
struct Eye {
  int cx, cy, w, h;
} L, R;

// ─── ②.a  LOW‑LEVEL OLED DRAW ROUTINES ────────────────────────────────
void drawEyes(bool flush = true)
{
  oled.clearDisplay();

  // left eye
  oled.fillRoundRect(L.cx - L.w / 2, L.cy - L.h / 2,
                     L.w, L.h, REF_RAD, SSD1306_WHITE);
  // right eye
  oled.fillRoundRect(R.cx - R.w / 2, R.cy - R.h / 2,
                     R.w, R.h, REF_RAD, SSD1306_WHITE);

  if (flush) oled.display();
}

void eyesCentre(bool flush = true)
{
  L.w = R.w = REF_EYE_W;
  L.h = R.h = REF_EYE_H;

  L.cx = OLED_WIDTH / 2 - REF_EYE_W / 2 - REF_SPACE / 2;
  R.cx = OLED_WIDTH / 2 + REF_EYE_W / 2 + REF_SPACE / 2;
  L.cy = R.cy = OLED_HEIGHT / 2;

  drawEyes(flush);
}

// ─── ②.b  EXPRESSIONS & MOTIONS ───────────────────────────────────────
void blinkOnce(uint8_t step = 12)
{
  for (int i = 0; i < 3; ++i) {           // close
    L.h -= step; R.h -= step; drawEyes(); delay(1);
  }
  for (int i = 0; i < 3; ++i) {           // open
    L.h += step; R.h += step; drawEyes(); delay(1);
  }
}

void blinkTwice() { blinkOnce(); delay(150); blinkOnce(); }

void happyEyes()
{
  eyesCentre(false);
  int off = REF_EYE_H / 2;
  for (int i = 0; i < 10; ++i) {
    oled.fillTriangle(L.cx - L.w / 2 - 1, L.cy + off,
                      L.cx + L.w / 2 + 1, L.cy + 5 + off,
                      L.cx - L.w / 2 - 1, L.cy + L.h + off,
                      SSD1306_BLACK);
    oled.fillTriangle(R.cx + R.w / 2 + 1, R.cy + off,
                      R.cx - R.w / 2 - 1, R.cy + 5 + off,
                      R.cx + R.w / 2 + 1, R.cy + R.h + off,
                      SSD1306_BLACK);
    off -= 2;
    oled.display(); delay(10);
  }
  delay(600);
}

void saccade(int dxSign, int dySign)
{
  const int dx = 8 * dxSign, dy = 6 * dySign;
  const int blinkA = 8;
  // out
  L.cx += dx; R.cx += dx; L.cy += dy; R.cy += dy;
  L.h -= blinkA; R.h -= blinkA; drawEyes(); delay(20);
  // settle
  L.h += blinkA; R.h += blinkA; drawEyes(); delay(20);
}

void eyeRoutine()
{
  // wake‑up open quickly
  for (int h = 2; h <= REF_EYE_H; h += 2) {
    L.h = R.h = h; drawEyes(); delay(8);
  }
  delay(120);

  eyesCentre();              delay(120);
  saccade(+1, 0);            delay(150);
  saccade(-1, 0);            delay(150);
  blinkTwice();              delay(150);
  happyEyes();               delay(150);
}

// ─── ③ SERVO HELPERS ──────────────────────────────────────────────────
void nodSequence()
{
  headServo.write(HEAD_CENTRE - HEAD_DELTA);
  blinkOnce();               delay(HEAD_PAUSE);

  headServo.write(HEAD_CENTRE + HEAD_DELTA);
                              delay(HEAD_PAUSE);

  headServo.write(HEAD_CENTRE);
                              delay(HEAD_PAUSE);
}

// ─── ④  ARDUINO STANDARD CALLBACKS ────────────────────────────────────
void setup()
{
  baseServo.attach(BASE_PIN);
  headServo.attach(HEAD_PIN);
  baseServo.write(BASE_CENTRE);
  headServo.write(HEAD_CENTRE);

  Wire.begin();
  oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  oled.clearDisplay();
  oled.setTextSize(2);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(2, OLED_HEIGHT / 2 - 8);
  oled.println(F("Hello!"));
  oled.setCursor(20, OLED_HEIGHT / 2 + 10);
  oled.println(F("I'm Riya"));
  oled.display();
  delay(3000);

  eyesCentre();              // eyes open
  delay(BASE_PAUSE);
}

void loop()
{
  // pan LEFT
  baseServo.write(BASE_CENTRE - BASE_DELTA);
  delay(BASE_PAUSE);
  nodSequence();
  eyeRoutine();

  // pan RIGHT
  baseServo.write(BASE_CENTRE + BASE_DELTA);
  delay(BASE_PAUSE);
  nodSequence();
  eyeRoutine();
}
