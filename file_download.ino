#include <WiFi.h>
#include <HTTPClient.h>
#include <LittleFS.h>
#include "ESPAsyncWebServer.h" 

const char* ssid = "";
const char* password = "";
const char* fileUrl = ""; // Replace with the URL of the large file you want to download

AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  if (!LittleFS.begin()) {
    Serial.println("Failed to mount LittleFS. Formatting filesystem....");
    LittleFS.format();
    Serial.println("Formatting done!. Please reset ESP32 ..");
    return;
  }

  downloadAndSaveFile(fileUrl, "/large_file.bin");
  
  server.on("/large_file.bin", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/large_file.bin", "text/html", false);
  });

  server.begin();

  Serial.print("Ge the file from http://");
  Serial.print(WiFi.localIP());
  Serial.println("/large_file.bin");
  
  Serial.println("All Done!");
}

void loop() {
  // Your main code here (if any)
}
 
void downloadAndSaveFile(const char* url, const char* filePath) {

  LittleFS.remove(filePath);  
  HTTPClient http;
  
  Serial.print("Downloading file: ");
  Serial.println(url);

  http.begin(url);
  int httpCode = http.GET();

  File file = LittleFS.open(filePath, "w");
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }

  if (httpCode == HTTP_CODE_OK) {
     // Get length of document (is -1 when Server sends no Content-Length header)
      int total = http.getSize();
      int len = total;

      // Create buffer for read
      uint8_t buff[128] = { 0 };

      // Get tcp stream
      WiFiClient * stream = http.getStreamPtr();

      // Read all data from server
      while (http.connected() && (len > 0 || len == -1)) {
        // Get available data size
        size_t size = stream->available();

        if (size) {
          // Read up to 128 bytes
          int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

          // Write it to file
          file.write(buff, c);

          // Calculate remaining bytes
          if (len > 0) {
            len -= c;
          }
        }
        yield();
      } 
  } else {
    Serial.printf("HTTP GET request failed with error code %d\n", httpCode);
  }

  file.close();
  http.end();

  // File size
  auto filea = LittleFS.open(filePath, "r");
  size_t filesize = filea.size();   
  Serial.print("Downloaded file size:");Serial.println(filesize);
  filea.close();
}
