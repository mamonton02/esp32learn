#include <Arduino.h>
#include "esp_log.h"
#include "esp_partition.h"
#include "esp_flash.h"

#ifndef GIT_HASH
#define GIT_HASH "UNKN"
#endif

extern esp_flash_t *esp_flash_default_chip;

static const uint32_t PARTITION_TABLE_ADDRESS = 0x8000;
static const size_t PARTITION_TABLE_SIZE = 0x1000; // 4096 bytes
static const size_t BYTES_PER_LINE = 16;

static const char *TAG = "LAB3_PARTITION_TABLE_DUMP";//"LAB3_PARTITIONS";// "LAB3_EXCEPTION";
static const char *FW_VERSION = "1.0.0";
static const char *BUILD_TIME = "2026-08-28";

// **
// -- Exceptions --
// **
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

// **
// -- Partitions --
// **

const char *partitionTypeToString(esp_partition_type_t type)
{
    switch (type)
    {
        case ESP_PARTITION_TYPE_APP:
            return "APP";

        case ESP_PARTITION_TYPE_DATA:
            return "DATA";

        default:
            return "UNKNOWN";
    }
}

const char *partitionSubtypeToString(esp_partition_type_t type, esp_partition_subtype_t subtype)
{
    if (type == ESP_PARTITION_TYPE_APP)
    {
        switch (subtype)
        {
            case ESP_PARTITION_SUBTYPE_APP_FACTORY:
                return "factory";

            case ESP_PARTITION_SUBTYPE_APP_OTA_0:
                return "ota_0";

            case ESP_PARTITION_SUBTYPE_APP_OTA_1:
                return "ota_1";

            case ESP_PARTITION_SUBTYPE_APP_OTA_2:
                return "ota_2";

            case ESP_PARTITION_SUBTYPE_APP_OTA_3:
                return "ota_3";

            case ESP_PARTITION_SUBTYPE_APP_OTA_4:
                return "ota_4";

            case ESP_PARTITION_SUBTYPE_APP_OTA_5:
                return "ota_5";

            case ESP_PARTITION_SUBTYPE_APP_OTA_6:
                return "ota_6";

            case ESP_PARTITION_SUBTYPE_APP_OTA_7:
                return "ota_7";

            case ESP_PARTITION_SUBTYPE_APP_OTA_8:
                return "ota_8";

            case ESP_PARTITION_SUBTYPE_APP_OTA_9:
                return "ota_9";

            case ESP_PARTITION_SUBTYPE_APP_OTA_10:
                return "ota_10";

            case ESP_PARTITION_SUBTYPE_APP_OTA_11:
                return "ota_11";

            case ESP_PARTITION_SUBTYPE_APP_OTA_12:
                return "ota_12";

            case ESP_PARTITION_SUBTYPE_APP_OTA_13:
                return "ota_13";

            case ESP_PARTITION_SUBTYPE_APP_OTA_14:
                return "ota_14";

            case ESP_PARTITION_SUBTYPE_APP_OTA_15:
                return "ota_15";

            case ESP_PARTITION_SUBTYPE_APP_TEST:
                return "test";

            default:
                return "app_unknown";
        }
    }

    if (type == ESP_PARTITION_TYPE_DATA)
    {
        switch (subtype)
        {
            case ESP_PARTITION_SUBTYPE_DATA_OTA:
                return "ota";

            case ESP_PARTITION_SUBTYPE_DATA_PHY:
                return "phy";

            case ESP_PARTITION_SUBTYPE_DATA_NVS:
                return "nvs";

            case ESP_PARTITION_SUBTYPE_DATA_COREDUMP:
                return "coredump";

            case ESP_PARTITION_SUBTYPE_DATA_NVS_KEYS:
                return "nvs_keys";

            case ESP_PARTITION_SUBTYPE_DATA_EFUSE_EM:
                return "efuse";

            case ESP_PARTITION_SUBTYPE_DATA_SPIFFS:
                return "spiffs";

            case ESP_PARTITION_SUBTYPE_DATA_FAT:
                return "fat";

            default:
                return "data_unknown";
        }
    }

    return "unknown";
}

void printPartitionTable()
{
    Serial.println();
    Serial.println("========== ESP32 Partition Table ==========");
    Serial.println("Label            Type       Subtype      Address      Size");
    Serial.println("-------------------------------------------------------------");

    esp_partition_iterator_t iterator = esp_partition_find(
        ESP_PARTITION_TYPE_ANY,
        ESP_PARTITION_SUBTYPE_ANY,
        NULL
    );

    esp_partition_iterator_t it = iterator;

    while (it != NULL)
    {
        const esp_partition_t *partition = esp_partition_get(it);

        if (partition != NULL)
        {
            char line[128];

            snprintf(
                line,
                sizeof(line),
                "%-16s %-10s %-12s 0x%08X   0x%06X",
                partition->label,
                partitionTypeToString(partition->type),
                partitionSubtypeToString(partition->type, partition->subtype),
                partition->address,
                partition->size
            );

            Serial.println(line);
        }

        it = esp_partition_next(it);
    }

    if (iterator != NULL)
    {
        esp_partition_iterator_release(iterator);
    }

    Serial.println("================================================");
    Serial.println();
}

void printHexDumpLine(const uint8_t *data, size_t length, uint32_t address)
{
    Serial.printf("0x%08X  ", address);

    // Hex bytes
    for (size_t i = 0; i < BYTES_PER_LINE; i++)
    {
        if (i < length)
        {
            Serial.printf("%02X ", data[i]);
        }
        else
        {
            Serial.print("   ");
        }
    }

    Serial.print(" ");

    // ASCII view
    for (size_t i = 0; i < length; i++)
    {
        uint8_t c = data[i];

        if (c >= 32 && c <= 126)
        {
            Serial.write(c);
        }
        else
        {
            Serial.print(".");
        }
    }

    Serial.println();
}

void dumpPartitionTableSection()
{
    Serial.println();
    Serial.println("========== Raw partition table flash dump ==========");
    Serial.printf("Address: 0x%08X\n", PARTITION_TABLE_ADDRESS);
    Serial.printf("Size   : 0x%X bytes\n", PARTITION_TABLE_SIZE);
    Serial.println("----------------------------------------------------");

    uint8_t buffer[BYTES_PER_LINE];

    for (uint32_t offset = 0; offset < PARTITION_TABLE_SIZE; offset += BYTES_PER_LINE)
    {
        esp_err_t err = esp_flash_read(
            esp_flash_default_chip,
            buffer,
            PARTITION_TABLE_ADDRESS + offset,
            BYTES_PER_LINE
        );

        if (err != ESP_OK)
        {
            Serial.printf(
                "esp_flash_read failed at flash address 0x%08X, error = 0x%X\n",
                PARTITION_TABLE_ADDRESS + offset,
                err
            );
            return;
        }

        printHexDumpLine(buffer, BYTES_PER_LINE, PARTITION_TABLE_ADDRESS + offset);
    }

    Serial.println("====================================================");
}

void setup() {
  Serial.begin(115200);

  Serial.println("Partition info checking...");

  ESP_LOGI(TAG, "fw %s build %s hash %s", FW_VERSION, BUILD_TIME, GIT_HASH);
  ESP_LOGI(TAG, "reset reason: %d", esp_reset_reason());
  ESP_LOGI(TAG, "free heap: %u", esp_get_free_heap_size());  

  delay(2000);

  //printPartitionTable();
  dumpPartitionTableSection();

  // Recommended reliable exception for the lab:
  // causeNullPointerException();

  // Alternative tests:
  // causeAbortException();
  // causeDivideByZero();

}

void loop() {}