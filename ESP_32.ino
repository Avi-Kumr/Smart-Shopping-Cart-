#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// WiFi Manager Class
class WiFiManager {
private:
    const char* ssid;
    const char* password;

public:
    WiFiManager(const char* ssid, const char* password) : ssid(ssid), password(password) {}

    void connect() {
        Serial.println("Connecting to WiFi...");
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {
            delay(1000);
            Serial.print(".");
        }
        Serial.println("\nConnected to WiFi");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    }
};

// Barcode Scanner Class
class BarcodeScanner {
private:
    String barcode;

public:
String readBarcode() {
    if (Serial.available() > 0) {
        String data = Serial.readStringUntil('\n');
        data.trim();
        return data; 
    }
    return "";
}

};

// HTTP Handler Class
class HTTPHandler {
private:
    String serverName;

public:
    HTTPHandler(const String& server) : serverName(server) {}

    bool fetchProductData(const String& barcode, StaticJsonDocument<256>& doc) {
        if (barcode.isEmpty() || WiFi.status() != WL_CONNECTED) return false;

        HTTPClient http;
        String url = serverName + "?barcode=" + barcode;
        http.begin(url);
        int httpResponseCode = http.GET();

        if (httpResponseCode > 0) {
            String response = http.getString();
            DeserializationError error = deserializeJson(doc, response);

            if (error) {
                Serial.print("JSON Parsing Failed: ");
                Serial.println(error.c_str());
                return false;
            }
            return true;
        } else {
            Serial.print("HTTP Request Error: ");
            Serial.println(httpResponseCode);
            return false;
        }
    }
};

// Main Program
WiFiManager wifiManager("Network SSID", "Network Password");
BarcodeScanner scanner;
HTTPHandler httpHandler("http://192.168.62.72:8080/items");

void setup() {
    Serial.begin(9600);
    Serial1.begin(9600);
    wifiManager.connect();
}

void loop() {
    String barcode = scanner.readBarcode();
    
    if (!barcode.isEmpty()) {
        StaticJsonDocument<256> doc;
        
        if (httpHandler.fetchProductData(barcode, doc)) {
            const char* name = doc[0]["name"];
            float price = doc[0]["price"];
            float weight = doc[0]["weight"];

            Serial.print("Sending to Arduino: Name: ");
            Serial.print(name);
            Serial.print(", Price: ");
            Serial.print(price);
            Serial.print(", Weight: ");
            Serial.println(weight);

            String dataToSend = String(name) + "," + String(price) + "," + String(weight);
            Serial1.println(dataToSend);
        }
    }

    delay(1000);
}
