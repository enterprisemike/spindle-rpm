#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

static const uint8_t SCREEN_WIDTH = 128;
static const uint8_t SCREEN_HEIGHT = 64;
static const int8_t OLED_RESET = -1;
static const uint8_t OLED_ADDRESS = 0x3C;

static const uint8_t HALL_SENSOR_PIN = 2;
static const uint8_t PULSES_PER_REV = 1;
static const uint32_t MIN_PULSE_INTERVAL_US = 800;
static const uint32_t SIGNAL_TIMEOUT_MS = 1200;
static const uint32_t DISPLAY_REFRESH_MS = 100;

static const uint16_t STARTUP_TARGET = 10000;
static const uint32_t STARTUP_DURATION_MS = 5000;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

volatile uint32_t lastPulseUs = 0;
volatile uint32_t pulseIntervalUs = 0;

uint32_t lastDisplayUpdateMs = 0;
float filteredRpm = 0.0f;

void onPulse()
{
  const uint32_t nowUs = micros();
  const uint32_t deltaUs = nowUs - lastPulseUs;

  if (deltaUs >= MIN_PULSE_INTERVAL_US) {
    pulseIntervalUs = deltaUs;
    lastPulseUs = nowUs;
  }
}

void drawCenteredLabel(const char *label)
{
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  int16_t x1;
  int16_t y1;
  uint16_t w;
  uint16_t h;
  display.getTextBounds(label, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, 4);
  display.print(label);
}

void drawRpmScreen(uint16_t rpm)
{
  display.clearDisplay();
  drawCenteredLabel("SPINDLE RPM");

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(3);

  char rpmText[8];
  snprintf(rpmText, sizeof(rpmText), "%u", rpm);

  int16_t x1;
  int16_t y1;
  uint16_t w;
  uint16_t h;
  display.getTextBounds(rpmText, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, 28);
  display.print(rpmText);

  display.display();
}

void runStartupAnimation()
{
  const uint32_t startMs = millis();

  while ((millis() - startMs) < STARTUP_DURATION_MS) {
    const uint32_t elapsedMs = millis() - startMs;
    const uint16_t value = (uint16_t)((elapsedMs * STARTUP_TARGET) / STARTUP_DURATION_MS);

    display.clearDisplay();
    drawCenteredLabel("STARTUP TEST");

    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);

    char countText[12];
    snprintf(countText, sizeof(countText), "%u", value);

    int16_t x1;
    int16_t y1;
    uint16_t w;
    uint16_t h;
    display.getTextBounds(countText, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((SCREEN_WIDTH - w) / 2, 30);
    display.print(countText);

    display.display();
    delay(25);
  }

  display.clearDisplay();
  drawCenteredLabel("READY");
  display.display();
  delay(300);
}

void setup()
{
  pinMode(HALL_SENSOR_PIN, INPUT_PULLUP);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    for (;;) {
      delay(100);
    }
  }

  display.clearDisplay();
  display.display();

  runStartupAnimation();

  attachInterrupt(digitalPinToInterrupt(HALL_SENSOR_PIN), onPulse, FALLING);
}

void loop()
{
  const uint32_t nowMs = millis();

  if ((nowMs - lastDisplayUpdateMs) < DISPLAY_REFRESH_MS) {
    return;
  }
  lastDisplayUpdateMs = nowMs;

  uint32_t intervalUsSnapshot;
  uint32_t lastPulseUsSnapshot;

  noInterrupts();
  intervalUsSnapshot = pulseIntervalUs;
  lastPulseUsSnapshot = lastPulseUs;
  interrupts();

  float rpm = 0.0f;

  if (intervalUsSnapshot > 0) {
    const uint32_t ageMs = (micros() - lastPulseUsSnapshot) / 1000;
    if (ageMs <= SIGNAL_TIMEOUT_MS) {
      rpm = (60000000.0f / intervalUsSnapshot) / PULSES_PER_REV;
    }
  }

  if (rpm <= 0.1f) {
    filteredRpm = 0.0f;
  } else if (filteredRpm <= 0.1f) {
    filteredRpm = rpm;
  } else {
    filteredRpm = (0.7f * filteredRpm) + (0.3f * rpm);
  }

  if (filteredRpm > 99999.0f) {
    filteredRpm = 99999.0f;
  }

  drawRpmScreen((uint16_t)filteredRpm);
}
