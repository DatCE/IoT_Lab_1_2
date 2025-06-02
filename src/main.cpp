// #define LED_PIN 48
// #define SDA_PIN GPIO_NUM_11
// #define SCL_PIN GPIO_NUM_12

// #include <WiFi.h>
// #include <Arduino_MQTT_Client.h>
// #include <ThingsBoard.h>
// #include "DHT20.h"
// #include "Wire.h"
// #include <ArduinoOTA.h>
// #include <HTTPClient.h>
// #include "cooperative_scheduler_O(1).h"
// #include "tensorflow/lite/micro/micro_interpreter.h"
// #include "model_data.h"

// constexpr char WIFI_SSID[] = "ACLAB";
// constexpr char WIFI_PASSWORD[] = "ACLAB2023";

// constexpr char TOKEN[] = "avll3m5wmw66wm7fink6";

// constexpr char THINGSBOARD_SERVER[] = "app.coreiot.io";
// constexpr uint16_t THINGSBOARD_PORT = 1883U;

// constexpr uint32_t MAX_MESSAGE_SIZE = 1024U;
// constexpr uint32_t SERIAL_DEBUG_BAUD = 115200U;

// constexpr char BLINKING_INTERVAL_ATTR[] = "blinkingInterval";
// constexpr char LED_MODE_ATTR[] = "ledMode";
// constexpr char LED_STATE_ATTR[] = "ledState";
// constexpr char LED_SCHEDULE[] = "Time";
// constexpr char FIRMWARE_URL[] = "fw_url";

// volatile bool attributesChanged = false;
// volatile int ledMode = 0;
// volatile bool ledState = false;
// volatile int ledOnTimeAfter = 0;
// volatile int ledOffTimeAfter = 0;

// bool OTA_UPDATE = false;

// constexpr uint16_t BLINKING_INTERVAL_MS_MIN = 10U;
// constexpr uint16_t BLINKING_INTERVAL_MS_MAX = 60000U;
// volatile uint16_t blinkingInterval = 1000U;
// TaskHandle_t TaskUpdateFirmwareHandle = NULL;

// uint32_t previousStateChange;

// constexpr int16_t telemetrySendInterval = 10000U;
// uint32_t previousDataSend;

// // OTA
// String previous_URL = "https://raw.githubusercontent.com/DatCE/IoT_Lab_1_2/Lab_3/firmware_prev.bin";
// String current_URL;
// //


// constexpr std::array<const char *, 4U> SHARED_ATTRIBUTES_LIST = {
//   LED_STATE_ATTR,
//   BLINKING_INTERVAL_ATTR,
//   LED_SCHEDULE,
//   FIRMWARE_URL
// };

// WiFiClient wifiClient;
// Arduino_MQTT_Client mqttClient(wifiClient);
// ThingsBoard tb(mqttClient, MAX_MESSAGE_SIZE);

// DHT20 dht20;

// RPC_Response setLedSwitchState(const RPC_Data &data) {
//     Serial.println("Received Switch state");
//     bool newState = data;
//     Serial.print("Switch state change: ");
//     Serial.println(newState);
//     digitalWrite(LED_PIN, newState);
//     attributesChanged = true;
//     return RPC_Response("setLedSwitchValue", newState);
// }

// const std::array<RPC_Callback, 1U> callbacks = {
//   RPC_Callback{ "setLedSwitchValue", setLedSwitchState }
// };

// void TaskBlinkingLED(void *pvParameters) {
//   const int maxBlinkCount = 10;
//   int counter = 0;
//   for (;;) {
//     if (counter >= maxBlinkCount) {
//       vTaskDelete(NULL);
//     }
//     digitalWrite(LED_PIN, HIGH);
//     vTaskDelay(blinkingInterval / portTICK_PERIOD_MS); 

//     digitalWrite(LED_PIN, LOW);
//     vTaskDelay(blinkingInterval / portTICK_PERIOD_MS); 

//     counter++;
//     Serial.print("Blinking count: ");
//     Serial.println(counter);
//   }
// }
// void TaskUpdateFirmware(void *pvParameters) {
//   for (;;) {
//     if (OTA_UPDATE)
//     {
//       OTA_UPDATE = false;
//       Serial.println("Checking for OTA update...");
//       HTTPClient http;
//       http.begin(current_URL);
//       int httpCode = http.GET();
  
//       if (httpCode == HTTP_CODE_OK) {
//           int contentLength = http.getSize();
//           if (contentLength <= 0) {
//               Serial.println("Error: Invalid content length");
//               tb.sendTelemetryData("fw_state", "Updated unsuccessfully");
//           } else {
//               WiFiClient* stream = http.getStreamPtr();
//               bool canBegin = Update.begin(contentLength);
  
//               if (canBegin) {
//                   Serial.println("Updating firmware...");
//                   size_t written = Update.writeStream(*stream);
//                   if (written == contentLength) {
//                       Serial.println("Update successful! Restarting...");
//                       tb.sendTelemetryData("fw_state", "Updated successfully");
//                       Update.end();
//                       ESP.restart();
//                   } else {
//                       Serial.println("Update failed. Bytes written: " + String(written));
//                       tb.sendTelemetryData("fw_state", "Updated unsuccessfully");
//                       Update.end();
//                   }
//               } else {
//                   Serial.println("Not enough space for update.");
//                   tb.sendTelemetryData("fw_state", "Updated unsuccessfully");
//               }
//           }
//       } else {
//           Serial.println("Failed to download firmware. HTTP code: " + String(httpCode));
//           tb.sendTelemetryData("fw_state", "Updated unsuccessfully");
//       }
      
//       http.end();
//     }
//     vTaskDelay(10000 / portTICK_PERIOD_MS);
//   }
// }

// void processSharedAttributes(const Shared_Attribute_Data &data) {
//   Serial.println("Processing shared attributes");
//   for (auto it = data.begin(); it != data.end(); ++it) {
//     if (strcmp(it->key().c_str(), BLINKING_INTERVAL_ATTR) == 0) {
//       const uint16_t new_interval = it->value().as<uint16_t>();
//       if (new_interval >= BLINKING_INTERVAL_MS_MIN && new_interval <= BLINKING_INTERVAL_MS_MAX) {
//         blinkingInterval = new_interval;
//         Serial.print("Blinking interval is set to: ");
//         Serial.println(new_interval);
//         xTaskCreate(TaskBlinkingLED, "TaskBlinkingLED", 4096, NULL, 2, NULL);
//       }
//     } 
//     if (strcmp(it->key().c_str(), LED_STATE_ATTR) == 0) {
//       ledState = it->value().as<bool>();
//       digitalWrite(LED_PIN, ledState);
//       Serial.print("LED state is set to: ");
//       Serial.println(ledState);
//     }
//     if (strcmp(it->key().c_str(), LED_SCHEDULE) == 0) {
//       Serial.print("LED_SCHEDULE: ");
//       Serial.println(it->value().as<String>());
//       ledOnTimeAfter  = data["led_on_after"].as<uint16_t>();   
//       ledOffTimeAfter = data["led_off_after"].as<uint16_t>(); 
//       Serial.print("ledOnTimeAfter: ");
//       Serial.println(ledOnTimeAfter);
//       Serial.print("ledOffTimeAfter: ");
//       Serial.println(ledOffTimeAfter);
//     } 
//     if (strcmp(it->key().c_str(), FIRMWARE_URL) == 0) {
//       current_URL = it->value().as<String>();
//       Serial.print("Firmware URL: ");
//       Serial.println(current_URL);
//       if (current_URL != previous_URL) {
//         Serial.println("Process update firmware");
//         previous_URL = current_URL;
//         OTA_UPDATE = true;
//       }
//       else
//       {
//         Serial.println("No update firmware");
//       }
//     } 
//   }
//   attributesChanged = true;
// }

// const Shared_Attribute_Callback attributes_callback(&processSharedAttributes, SHARED_ATTRIBUTES_LIST.cbegin(), SHARED_ATTRIBUTES_LIST.cend());
// const Attribute_Request_Callback attribute_shared_request_callback(&processSharedAttributes, SHARED_ATTRIBUTES_LIST.cbegin(), SHARED_ATTRIBUTES_LIST.cend());

// void InitWiFi() {
//   Serial.println("Connecting to AP ...");
//   // Attempting to establish a connection to the given WiFi network
//   WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
//   while (WiFi.status() != WL_CONNECTED) {
//     // Delay 500ms until a connection has been successfully established
//     delay(500);
//     Serial.print(".");
//   }
//   Serial.println("Connected to AP");
// }

// const bool reconnect() {
//   // Check to ensure we aren't connected yet
//   const wl_status_t status = WiFi.status();
//   if (status == WL_CONNECTED) {
//     return true;
//   }
//   // If we aren't establish a new connection to the given WiFi network
//   InitWiFi();
//   return true;
// }


// void TaskWifiCheck(void *pvParameters)
// {
//   for (;;)
//   {
//     if (!reconnect()) {
//       return;
//     }
//     vTaskDelay(1000 / portTICK_PERIOD_MS);
//   }
// }

// void TaskCoreIOTCheck(void *pvParameters)
// {
//   for (;;)
//   {
//     if (!tb.connected()) {
//       Serial.print("Connecting to: ");
//       Serial.print(THINGSBOARD_SERVER);
//       Serial.print(" with token ");
//       Serial.println(TOKEN);
//       if (!tb.connect(THINGSBOARD_SERVER, TOKEN, THINGSBOARD_PORT)) {
//         Serial.println("Failed to connect");
//         return;
//       }
  
//       tb.sendAttributeData("macAddress", WiFi.macAddress().c_str());
  
//       Serial.println("Subscribing for RPC...");
//       if (!tb.RPC_Subscribe(callbacks.cbegin(), callbacks.cend())) {
//         Serial.println("Failed to subscribe for RPC");
//         return;
//       }
  
//       if (!tb.Shared_Attributes_Subscribe(attributes_callback)) {
//         Serial.println("Failed to subscribe for shared attribute updates");
//         return;
//       }
  
//       Serial.println("Subscribe done");
  
//       if (!tb.Shared_Attributes_Request(attribute_shared_request_callback)) {
//         Serial.println("Failed to request for shared attributes");
//         return;
//       }
//     }
//     vTaskDelay(1000 / portTICK_PERIOD_MS);
//   }
// }

// void TaskSendTelemetry(void *pvParameters)
// {
//   for (;;)
//   {
//     dht20.read();
    
//     float temperature = dht20.getTemperature();
//     float humidity = dht20.getHumidity();

  
//     if (isnan(temperature) || isnan(humidity)) {
//       Serial.println("Failed to read from DHT20 sensor!");
//     } else {
//       // Serial.println("----- New OTA -----");
//       Serial.print("Temperature: ");
//       Serial.print(temperature);
//       Serial.print(" °C, Humidity: ");
//       Serial.print(humidity);
//       Serial.println(" %");
  
//       tb.sendTelemetryData("temperature", temperature);
//       tb.sendTelemetryData("humidity", humidity);
//     }
//     vTaskDelay(5000 / portTICK_PERIOD_MS);
//   }
// }

// void TaskSendAttributes(void *pvParameters)
// {
//   for (;;)
//   {
//     if (attributesChanged) {
//       attributesChanged = false;
//       tb.sendAttributeData(LED_STATE_ATTR, digitalRead(LED_PIN));
//     }
//     tb.sendAttributeData("rssi", WiFi.RSSI());
//     tb.sendAttributeData("channel", WiFi.channel());
//     tb.sendAttributeData("bssid", WiFi.BSSIDstr().c_str());
//     tb.sendAttributeData("localIp", WiFi.localIP().toString().c_str());
//     tb.sendAttributeData("ssid", WiFi.SSID().c_str());
//     vTaskDelay(1000 / portTICK_PERIOD_MS);
//   }
// }

// void TaskTBLoop (void *pvParameters)
// {
//   for (;;)
//   {
//     tb.loop();
//     vTaskDelay(10 / portTICK_PERIOD_MS);
//   }
// }
// void setup() {
//   Serial.begin(SERIAL_DEBUG_BAUD);
//   pinMode(LED_PIN, OUTPUT);
//   delay(1000);
//   InitWiFi();

//   Wire.begin(SDA_PIN, SCL_PIN);
//   dht20.begin();
//   xTaskCreate(TaskWifiCheck, "TaskWifiCheck", 4096, NULL, 2, NULL);
//   xTaskCreate(TaskCoreIOTCheck, "TaskCoreIOTCheck", 4096, NULL, 2, NULL);
//   xTaskCreate(TaskSendTelemetry, "TaskSendTelemetry", 4096, NULL, 2, NULL);
//   xTaskCreate(TaskSendAttributes, "TaskSendAttributes", 4096, NULL, 2, NULL);
//   xTaskCreate(TaskTBLoop, "TaskTBLoop", 8192, NULL, 2, NULL);
//   xTaskCreate(TaskUpdateFirmware, "TaskUpdateFirmware", 8192, NULL, 2, NULL);
  
// }

// void loop() {
    
// }

// #define LED_PIN 48
// #define SDA_PIN GPIO_NUM_11
// #define SCL_PIN GPIO_NUM_12

// #include <WiFi.h>
// #include <Arduino_MQTT_Client.h>
// #include <ThingsBoard.h>
// #include "DHT20.h"
// #include "Wire.h"
// #include <ArduinoOTA.h>
// #include <HTTPClient.h>
// #include "cooperative_scheduler_O(1).h"

// #include <Arduino.h>
// #include "tensorflow/lite/schema/schema_generated.h"
// #include "tensorflow/lite/micro/micro_interpreter.h"
// #include "tensorflow/lite/micro/all_ops_resolver.h"
// #include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
// #include "model_data.cc"

// const tflite::Model* model = nullptr;
// tflite::MicroInterpreter* interpreter = nullptr;
// TfLiteTensor* input = nullptr;
// TfLiteTensor* output = nullptr;

// // Bộ nhớ dùng cho mô hình
// constexpr int tensor_arena_size = 10 * 1024;
// uint8_t tensor_arena[tensor_arena_size];

// void setup() {
//   Serial.begin(115200);

//   // Load mô hình từ mảng byte
//   model = tflite::GetModel(model_tflite);
//   if (model->version() != TFLITE_SCHEMA_VERSION) {
//     Serial.println("Model version mismatch!");
//     while (1); // Dừng nếu sai version
//   }

//   // Load tất cả các toán tử (layers)
//   static tflite::AllOpsResolver resolver;

//   // Khởi tạo interpreter
//   static tflite::MicroInterpreter static_interpreter(
//     model, resolver, tensor_arena, tensor_arena_size);
//   interpreter = &static_interpreter;

//   // Cấp phát bộ nhớ cho tensor
//   interpreter->AllocateTensors();

//   // Lấy con trỏ tới input/output tensor
//   input = interpreter->input(0);
//   output = interpreter->output(0);
// }

// void loop() {
//   // Giả sử bạn có 10 giá trị float đầu vào từ cảm biến hoặc test
//   float input_data[10] = {23.1, 23.2, 23.4, 23.7, 24.0, 24.2, 24.5, 24.7, 25.0, 25.3};

//   // Gán dữ liệu vào tensor đầu vào
//   for (int i = 0; i < 10; ++i) {
//     input->data.f[i] = input_data[i];
//   }

//   // Chạy inference
//   TfLiteStatus invoke_status = interpreter->Invoke();
//   if (invoke_status != kTfLiteOk) {
//     Serial.println("Dự đoán thất bại!");
//     return;
//   }

//   // Lấy kết quả
//   float result = output->data.f[0];
//   Serial.print("Dự đoán giá trị tiếp theo: ");
//   Serial.println(result);

//   delay(5000); // chờ 5s rồi dự đoán lại
// }







//////////////////////////////////////////////



#include <Chirale_TensorFlowLite.h>

// include static array definition of pre-trained model
#include "model.h"

// This TensorFlow Lite Micro Library for Arduino is not similar to standard
// Arduino libraries. These additional header files must be included.
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"

// Globals pointers, used to address TensorFlow Lite components.
// Pointers are not usual in Arduino sketches, future versions of
// the library may change this...
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;
const float max_val = 26.70200963609803;
const float min_val = 23.218951650506252;
// There is no way to calculate this parameter
// the value is usually determined by trial and errors
// It is the dimension of the memory area used by the TFLite interpreter
// to store tensors and intermediate results
constexpr int kTensorArenaSize = 20000;

// Keep aligned to 16 bytes for CMSIS (Cortex Microcontroller Software Interface Standard)
// alignas(16) directive is used to specify that the array 
// should be stored in memory at an address that is a multiple of 16.
alignas(16) uint8_t tensor_arena[kTensorArenaSize];


void setup() {
  // Initialize serial communications and wait for Serial Monitor to be opened
  Serial.begin(9600);
  unsigned long startTime = millis();
  Serial.println("Waiting for Serial to be ready...");
  while (!Serial) {
    if (millis() - startTime > 5000) { // 5000 ms = 5 giây
      Serial.println("Timeout waiting for Serial.");
      break; // thoát vòng chờ
    }
  }

  Serial.println("Sine(x) function inference example.");
  Serial.println("Initializing TensorFlow Lite Micro Interpreter...");

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(g_model);

  // Check if model and library have compatible schema version,
  // if not, there is a misalignement between TensorFlow version used
  // to train and generate the TFLite model and the current version of library
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model provided and schema version are not equal!");
    while(true)
    {
      Serial.println("ERROR: Model version mismatch! Stuck here.");
      delay(1000);
    } // stop program here
  }

  // This pulls in all the TensorFlow Lite operators.
  static tflite::AllOpsResolver resolver;

  // Build an interpreter to run the model with.
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize);
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors.
  // if an error occurs, stop the program.
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    Serial.println("AllocateTensors() failed");
    while(true)
    {
      Serial.println("ERROR: AllocateTensors failed! Stuck here.");
      delay(1000);
    } // stop program here
  }

  // Obtain pointers to the model's input and output tensors.
  input = interpreter->input(0);
  output = interpreter->output(0);

  Serial.println("Initialization done.");
  Serial.println("");
  Serial.println("Please, input a float number between 0 and 6.28");
}

void loop() {
  // 1. Tạo ngẫu nhiên 10 giá trị quanh 25 (ví dụ: từ 23.5 đến 26.5)
  float input_data[10];
  for (int i = 0; i < 10; ++i) {
    input_data[i] = 25.0 + random(-150, 151) / 100.0;  // -1.5 đến +1.5
  }

  // In input
  Serial.print("Input: ");
  for (int i = 0; i < 10; ++i) {
    Serial.print(input_data[i], 2);
    Serial.print(" ");
  }
  Serial.println();

  // 2. Gán giá trị float trực tiếp vào input tensor (float32)
  for (int i = 0; i < 10; ++i) {
    // Scale input_data[i] về [0,1] rồi mới gán vào input tensor
    input->data.f[i] = (input_data[i] - min_val) / (max_val - min_val);
  }
  for (int i = 0; i < 10; ++i) {
    Serial.print(input->data.f[i], 2);
    Serial.print(" ");
  }
  // 3. Dự đoán
  if (interpreter->Invoke() != kTfLiteOk) {
    Serial.println("Invoke failed!");
    delay(1000);
    return;
  }

  // 4. Lấy output float trực tiếp
  float y = output->data.f[0];

  // after scale (nếu bạn có scale riêng, giữ nguyên phần này)
  y = y * (max_val - min_val) + min_val;

  Serial.print("Predicted value 11: ");
  Serial.println(y, 2);

  delay(2000);
}

