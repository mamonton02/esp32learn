#include <Arduino.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Most LILYGO LoRa32 boards use I2C address 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// TTGO LoRa32 T3 V1.6.1 LoRa (SX1276/78) wiring
static const int PIN_SCK  = 5;    // LoRa2 / SCK
static const int PIN_MOSI = 27;   // LoRa1 / MOSI
static const int PIN_MISO = 19;   // MISO
static const int PIN_NSS  = 18;   // CS / NSS
static const int PIN_RST  = 23;   // RST (optional but recommended)

static inline void spiDelay() {
  // adjust if needed; start small. On ESP32 even a few us is enough.
  delayMicroseconds(2);
}

static void spiBeginPins() {
  pinMode(PIN_SCK, OUTPUT);
  pinMode(PIN_MOSI, OUTPUT);
  pinMode(PIN_MISO, INPUT);
  pinMode(PIN_NSS, OUTPUT);

  digitalWrite(PIN_NSS, HIGH);   // deselect
  digitalWrite(PIN_SCK, LOW);    // CPOL=0 idle low
  digitalWrite(PIN_MOSI, LOW);
}

static uint8_t spiTransferByte(uint8_t outByte) {
  uint8_t inByte = 0;

  // MSB first
  for (int i = 7; i >= 0; --i) {
    // prepare MOSI while clock is low
    digitalWrite(PIN_MOSI, (outByte & (1 << i)) ? HIGH : LOW);
    spiDelay();

    // rising edge: slave samples MOSI, master samples MISO (mode 0)
    digitalWrite(PIN_SCK, HIGH);
    spiDelay();

    inByte <<= 1;
    if (digitalRead(PIN_MISO)) inByte |= 1;

    // falling edge: finish cycle
    digitalWrite(PIN_SCK, LOW);
    spiDelay();
  }

  return inByte;
}

static uint8_t sx1276ReadReg(uint8_t addr) {
  digitalWrite(PIN_NSS, LOW);
  spiDelay();

  spiTransferByte(addr | 0x80);     // READ command
  uint8_t val = spiTransferByte(0); // clock out data

  digitalWrite(PIN_NSS, HIGH);
  spiDelay();

  return val;
}

static void sx1276ResetPulse() {
  pinMode(PIN_RST, OUTPUT);
  digitalWrite(PIN_RST, LOW);
  delay(2);
  digitalWrite(PIN_RST, HIGH);
  delay(10);
}

void setup() {
  Serial.begin(115200);
  
  Wire.begin(21, 22); // SDA=21, SCL=22 (per your pinout)

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    while (true) delay(100);
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("RegVer:");
  display.display();
  delay(500);
  

  spiBeginPins();
  sx1276ResetPulse(); // optional but recommended

  uint8_t version = sx1276ReadReg(0x42);

  Serial.print("SX1276 RegVersion (0x42) = 0x");
  if (version < 0x10) Serial.print("0");
  Serial.println(version, HEX);

 display.setTextSize(2);
  display.setCursor(0, 32);
  display.print("0x");
  if (version < 0x10) display.print("0");
  display.print(version, HEX);
  display.display(); 
}

void loop() {}