#define LED_PIN 48
#define SDA_PIN GPIO_NUM_11
#define SCL_PIN GPIO_NUM_12

#include <WiFi.h>
#include <Arduino_MQTT_Client.h>
#include <ThingsBoard.h>
#include "DHT20.h"
#include "Wire.h"
#include <ArduinoOTA.h>

constexpr char WIFI_SSID[] = "Duc Dat";
constexpr char WIFI_PASSWORD[] = "03012013";

constexpr char TOKEN[] = "48fqddkes9mbys5hgu1g";

constexpr char THINGSBOARD_SERVER[] = "app.coreiot.io";
constexpr uint16_t THINGSBOARD_PORT = 1883U;

constexpr uint32_t MAX_MESSAGE_SIZE = 1024U;
constexpr uint32_t SERIAL_DEBUG_BAUD = 115200U;

constexpr char BLINKING_INTERVAL_ATTR[] = "blinkingInterval";
constexpr char LED_MODE_ATTR[] = "ledMode";
constexpr char LED_STATE_ATTR[] = "ledState";

volatile bool attributesChanged = false;
volatile int ledMode = 0;
volatile bool ledState = false;

constexpr uint16_t BLINKING_INTERVAL_MS_MIN = 10U;
constexpr uint16_t BLINKING_INTERVAL_MS_MAX = 60000U;
volatile uint16_t blinkingInterval = 1000U;

uint32_t previousStateChange;

constexpr int16_t telemetrySendInterval = 10000U;
uint32_t previousDataSend;

constexpr std::array<const char *, 2U> SHARED_ATTRIBUTES_LIST = {
  LED_STATE_ATTR,
  BLINKING_INTERVAL_ATTR
};

WiFiClient wifiClient;
Arduino_MQTT_Client mqttClient(wifiClient);
ThingsBoard tb(mqttClient, MAX_MESSAGE_SIZE);

DHT20 dht20;

RPC_Response setLedSwitchState(const RPC_Data &data) {
    Serial.println("Received Switch state");
    bool newState = data;
    Serial.print("Switch state change: ");
    Serial.println(newState);
    digitalWrite(LED_PIN, newState);
    attributesChanged = true;
    return RPC_Response("setLedSwitchValue", newState);
}

const std::array<RPC_Callback, 1U> callbacks = {
  RPC_Callback{ "setLedSwitchValue", setLedSwitchState }
};

void processSharedAttributes(const Shared_Attribute_Data &data) {
  for (auto it = data.begin(); it != data.end(); ++it) {
    if (strcmp(it->key().c_str(), BLINKING_INTERVAL_ATTR) == 0) {
      const uint16_t new_interval = it->value().as<uint16_t>();
      if (new_interval >= BLINKING_INTERVAL_MS_MIN && new_interval <= BLINKING_INTERVAL_MS_MAX) {
        blinkingInterval = new_interval;
        Serial.print("Blinking interval is set to: ");
        Serial.println(new_interval);
      }
    } else if (strcmp(it->key().c_str(), LED_STATE_ATTR) == 0) {
      ledState = it->value().as<bool>();
      digitalWrite(LED_PIN, ledState);
      Serial.print("LED state is set to: ");
      Serial.println(ledState);
    }
  }
  attributesChanged = true;
}

const Shared_Attribute_Callback attributes_callback(&processSharedAttributes, SHARED_ATTRIBUTES_LIST.cbegin(), SHARED_ATTRIBUTES_LIST.cend());
const Attribute_Request_Callback attribute_shared_request_callback(&processSharedAttributes, SHARED_ATTRIBUTES_LIST.cbegin(), SHARED_ATTRIBUTES_LIST.cend());

void InitWiFi() {
  Serial.println("Connecting to AP ...");
  // Attempting to establish a connection to the given WiFi network
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    // Delay 500ms until a connection has been successfully established
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}

const bool reconnect() {
  // Check to ensure we aren't connected yet
  const wl_status_t status = WiFi.status();
  if (status == WL_CONNECTED) {
    return true;
  }
  // If we aren't establish a new connection to the given WiFi network
  InitWiFi();
  return true;
}

void TaskWifiCheck(void *pvParameters)
{
  for (;;)
  {
    if (!reconnect()) {
      return;
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void TaskCoreIOTCheck(void *pvParameters)
{
  for (;;)
  {
    if (!tb.connected()) {
      Serial.print("Connecting to: ");
      Serial.print(THINGSBOARD_SERVER);
      Serial.print(" with token ");
      Serial.println(TOKEN);
      if (!tb.connect(THINGSBOARD_SERVER, TOKEN, THINGSBOARD_PORT)) {
        Serial.println("Failed to connect");
        return;
      }
  
      tb.sendAttributeData("macAddress", WiFi.macAddress().c_str());
  
      Serial.println("Subscribing for RPC...");
      if (!tb.RPC_Subscribe(callbacks.cbegin(), callbacks.cend())) {
        Serial.println("Failed to subscribe for RPC");
        return;
      }
  
      if (!tb.Shared_Attributes_Subscribe(attributes_callback)) {
        Serial.println("Failed to subscribe for shared attribute updates");
        return;
      }
  
      Serial.println("Subscribe done");
  
      if (!tb.Shared_Attributes_Request(attribute_shared_request_callback)) {
        Serial.println("Failed to request for shared attributes");
        return;
      }
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void TaskSendTelemetry(void *pvParameters)
{
  for (;;)
  {
    dht20.read();
    
    float temperature = dht20.getTemperature();
    float humidity = dht20.getHumidity();
  
    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Failed to read from DHT20 sensor!");
    } else {
      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.print(" °C, Humidity: ");
      Serial.print(humidity);
      Serial.println(" %");
  
      tb.sendTelemetryData("temperature", temperature);
      tb.sendTelemetryData("humidity", humidity);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void TaskSendAttributes(void *pvParameters)
{
  for (;;)
  {
    if (attributesChanged) {
      attributesChanged = false;
      tb.sendAttributeData(LED_STATE_ATTR, digitalRead(LED_PIN));
    }
    tb.sendAttributeData("rssi", WiFi.RSSI());
    tb.sendAttributeData("channel", WiFi.channel());
    tb.sendAttributeData("bssid", WiFi.BSSIDstr().c_str());
    tb.sendAttributeData("localIp", WiFi.localIP().toString().c_str());
    tb.sendAttributeData("ssid", WiFi.SSID().c_str());
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void TaskTBLoop (void *pvParameters)
{
  for (;;)
  {
    tb.loop();
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}
void setup() {
  Serial.begin(SERIAL_DEBUG_BAUD);
  pinMode(LED_PIN, OUTPUT);
  delay(1000);
  InitWiFi();

  Wire.begin(SDA_PIN, SCL_PIN);
  dht20.begin();
  xTaskCreate(TaskWifiCheck, "TaskWifiCheck", 2048, NULL, 2, NULL);
  xTaskCreate(TaskCoreIOTCheck, "TaskCoreIOTCheck", 2048, NULL, 2, NULL);
  xTaskCreate(TaskSendTelemetry, "TaskSendTelemetry", 2048, NULL, 2, NULL);
  xTaskCreate(TaskSendAttributes, "TaskSendAttributes", 2048, NULL, 2, NULL);
  xTaskCreate(TaskTBLoop, "TaskTBLoop", 2048, NULL, 2, NULL);
  
}

void loop() {
    
}

// #include <Arduino.h>
// #include <Wire.h>
// #include "DHT20.h"

// // Task handles
// TaskHandle_t Task1Handle = NULL;
// TaskHandle_t Task2Handle = NULL;
// TaskHandle_t Task3Handle = NULL;

// // DHT20 Sensor
// DHT20 DHT;

// void Task1(void *pvParameters) {
//     while (1) {
//         Serial.println("Hello from Task1");
//         vTaskDelay(1000);  // Delay 1000ms
//     }
// }

// void Task2(void *pvParameters) {
//     while (1) {
//         Serial.println("Hello from Task2");
//         vTaskDelay(1500);  // Delay 1500ms
//     }
// }

// // Task 3: Read DHT20 Temperature & Humidity
// void Task3(void *pvParameters) {
//     while (1) {
//         if (millis() - DHT.lastRead() >= 2000) {
//             int status = DHT.read();

//             Serial.print("DHT20 Temperature: ");
//             Serial.print(DHT.getTemperature(), 1);
//             Serial.println(" °C");

//             Serial.print("DHT20 Humidity: ");
//             Serial.print(DHT.getHumidity(), 1);
//             Serial.println(" %");

//             Serial.print("Status: ");
//             switch (status) {
//                 case DHT20_OK:
//                     Serial.println("OK");
//                     break;
//                 case DHT20_ERROR_CHECKSUM:
//                     Serial.println("Checksum error");
//                     break;
//                 case DHT20_ERROR_CONNECT:
//                     Serial.println("Connect error");
//                     break;
//                 case DHT20_MISSING_BYTES:
//                     Serial.println("Missing bytes");
//                     break;
//                 case DHT20_ERROR_BYTES_ALL_ZERO:
//                     Serial.println("All bytes read zero");
//                     break;
//                 case DHT20_ERROR_READ_TIMEOUT:
//                     Serial.println("Read time out");
//                     break;
//                 case DHT20_ERROR_LASTREAD:
//                     Serial.println("Read too fast");
//                     break;
//                 default:
//                     Serial.println("Unknown error");
//                     break;
//             }
//             Serial.println();
//         }
//         vTaskDelay(2000);  // Delay 2000ms
//     }
// }

// void setup() {
//     Serial.begin(115200);
//     // Wire.begin(GPIO_NUM_11, GPIO_NUM_12);
//     Wire.begin();

//     // Initialize DHT20
//     // if (!DHT.begin()) {
//     //     Serial.println("Failed to initialize DHT20 sensor!");
//     //     while (1);
//     // }
//     Serial.println("DHT20 sensor initialized.");

//     // Create Tasks
//     xTaskCreate(Task1, "Task1", 2048, NULL, 2, NULL);
//     xTaskCreate(Task2, "Task2", 2048, NULL, 2, NULL);
//     xTaskCreate(Task3, "DHT20Task", 2048, NULL, 2, NULL);
// }

// void loop() {
//     // Empty - FreeRTOS handles tasks
//     Serial.println("Hello guys");
//     delay(100);
// }