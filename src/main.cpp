#include <Arduino.h>
#include "esp_log.h"

#ifndef GIT_HASH
#define GIT_HASH "UNKN"
#endif

#include <Arduino.h>
#include "esp_log.h"

static const char *TAG = "LAB3_EXCEPTION";
static const char *FW_VERSION = "1.0.0";
static const char *BUILD_TIME = "2026-08-28";

void causeNullPointerException()
{
    ESP_LOGI(TAG, "About to cause null pointer exception...");

    delay(100);

    volatile int *ptr = nullptr;

    // This should cause a StoreProhibited exception on ESP32.
    // The ESP32 panic handler will print:
    // - Guru Meditation Error
    // - exception cause
    // - CPU registers
    // - backtrace
    *ptr = 123;
}

void causeAbortException()
{
    ESP_LOGI(TAG, "About to call abort()...");

    delay(100);

    // This intentionally aborts the program.
    // It also prints a panic/backtrace, but the cause will be "abort() was called",
    // not a CPU divide-by-zero exception.
    abort();
}

void causeDivideByZero()
{
    ESP_LOGI(TAG, "About to divide by zero...");

    delay(100);

    volatile int a = 10;
    volatile int b = 0;

    // Warning:
    // In C/C++, integer division by zero is undefined behavior.
    // On ESP32 it may cause an exception, reboot, or behave unexpectedly.
    volatile int c = a / b;

    ESP_LOGI(TAG, "Result = %d", c);
}


void setup() {
  Serial.begin(115200);

  Serial.println("Exception test started (again)...");

  ESP_LOGI(TAG, "fw %s build %s hash %s", FW_VERSION, BUILD_TIME, GIT_HASH);
  ESP_LOGI(TAG, "reset reason: %d", esp_reset_reason());
  ESP_LOGI(TAG, "free heap: %u", esp_get_free_heap_size());  

  delay(2000);

  // Recommended reliable exception for the lab:
  causeNullPointerException();

  // Alternative tests:
  // causeAbortException();
  // causeDivideByZero();

}

void loop() {}