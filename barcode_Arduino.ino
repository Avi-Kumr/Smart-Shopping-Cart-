#include <Adafruit_GFX.h>
#include <Adafruit_TFTLCD.h>
#include <TouchScreen.h>
#include <string.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <MFRC522.h>
#include "HX711.h"

// Constants
const int maxItems = 20;

// Pin configurations
#define YP A2
#define XM A3
#define YM 22
#define XP 23
#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4
#define SS_PIN 53
#define RST_PIN 5
#define DOUT 3
#define CLK 2

// Touch screen calibration
#define TS_MINX 158
#define TS_MINY 127
#define TS_MAXX 895
#define TS_MAXY 872
#define MINPRESSURE 10
#define MAXPRESSURE 1000

// Colors
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF
#define ORANGE 0xFD20
#define DARKORANGE 0xFB60
#define MAROON 0x7800
#define BLACKM 0x18E3

// Button positions
int BtnGreenX = 290;
int BtnGreenY = 30;
int BtnRedX = 290;
int BtnRedY = 165;

// Global variables
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
HX711 scale;
MFRC522 rfid(SS_PIN, RST_PIN);
byte authorizedUID[] = {0xB3, 0xBD, 0xD8, 0x16};

// Item structure
struct Item {
  String barcode;
  String name;
  float price;
  float weight;
};

// ShoppingCart class
class ShoppingCart {
private:
  Item items[maxItems];
  int itemCount = 0;
  float totalAmount = 0.0;
  float totalWeight = 0.0;

public:
  void addItem(String barcode, String name, float price, float weight) {
    if (itemCount < maxItems) {
      items[itemCount] = {barcode, name, price, weight};
      itemCount++;
      updateTotals();
    }
  }

  void removeItem(String barcode) {
    for (int i = 0; i < itemCount; i++) {
      if (items[i].barcode == barcode) {
        for (int j = i; j < itemCount - 1; j++) {
          items[j] = items[j + 1];
        }
        itemCount--;
        updateTotals();
        break;
      }
    }
  }

  void updateTotals() {
    totalAmount = 0.0;
    totalWeight = 0.0;
    for (int i = 0; i < itemCount; i++) {
      totalAmount += items[i].price;
      totalWeight += items[i].weight;
    }
  }

  float getTotalAmount() { return totalAmount; }
  float getTotalWeight() { return totalWeight; }
  int getItemCount() { return itemCount; }
  Item getItem(int index) { return items[index]; }
};

// DisplayManager class
class DisplayManager {
private:
  Adafruit_TFTLCD &tft;

public:
  DisplayManager(Adafruit_TFTLCD &tft) : tft(tft) {}

  void drawMenu() {
    tft.fillScreen(WHITE);
    tft.setTextSize(2);
    tft.setTextColor(BLACK);
    tft.setCursor(30, 40);
    tft.print("Government Engineering College Vaishali");
    tft.setCursor(50, 100);
    tft.setTextColor(MAROON);
    tft.print("Smart Shopping Cart");
    drawButtonNewTransaction();
  }

  void drawButtonNewTransaction() {
    tft.fillRect(65, 180, 190, 35, RED);
    tft.drawRect(65, 180, 190, 35, BLACK);
    tft.setCursor(60, 190);
    tft.setTextColor(WHITE);
    tft.print(" NEW TRANSACTION");
  }

  void drawProductList(ShoppingCart &cart, int scrollCounter) {
    tft.fillRect(2, 25, 317, 170, WHITE);
    tft.drawRect(2, 25, 317, 170, BLACK);
    int counterItems = 0;
    for (int l = scrollCounter; l < cart.getItemCount(); l++) {
      tft.setCursor(5, 30 + (25 * counterItems));
      tft.print(cart.getItem(l).name);
      tft.setCursor(205, 30 + (25 * counterItems));
      tft.print(cart.getItem(l).price);
      counterItems++;
      if (counterItems >= 5) break;
    }
  }

  void drawVerifyCheckOut(float totalAmount, String paymentStatus) {
    tft.fillScreen(WHITE);
    tft.setCursor(60, 15);
    tft.setTextColor(BLACK);
    tft.setTextSize(2);
    tft.print("Verify Check-Out");
    tft.drawLine(0, 40, 320, 40, BLACK);
    tft.setCursor(40, 50);
    tft.print("Bill Amount:");
    tft.setCursor(195, 50);
    tft.print(totalAmount);
    tft.setTextSize(2);
    tft.setCursor(5, 80);
    tft.print("Please pay at the counter.");
    tft.setCursor(50, 150);
    tft.print("Status: ");
    tft.setCursor(140, 150);
    tft.print(paymentStatus);
    drawBack();
  }

  void drawBack() {
    tft.fillRoundRect(80, 200, 150, 30, 5, BLACKM);
    tft.fillRoundRect(82, 202, 146, 26, 5, RED);
    tft.setTextSize(2);
    tft.setTextColor(BLACKM);
    tft.setCursor(115 + 20, 200 + 8);
    tft.print("Back");
  }

  void drawWeightMismatch() {
    tft.fillScreen(WHITE);
    tft.setCursor(60, 80);
    tft.setTextColor(BLACK);
    tft.setTextSize(2);
    tft.print("Weight mismatch");
  }
};

// RFIDManager class
class RFIDManager {
private:
  MFRC522 &rfid;
  byte *authorizedUID;

public:
  RFIDManager(MFRC522 &rfid, byte *authorizedUID) : rfid(rfid), authorizedUID(authorizedUID) {}

  bool isAuthorized() {
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
      for (byte i = 0; i < rfid.uid.size; i++) {
        if (rfid.uid.uidByte[i] != authorizedUID[i]) {
          return false;
        }
      }
      return true;
    }
    return false;
  }
};

// Global instances
ShoppingCart cart;
DisplayManager display(tft);
RFIDManager rfidManager(rfid, authorizedUID);

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  scale.begin(DOUT, CLK);
  scale.set_scale(461.248382);
  scale.tare();
  tft.reset();
  tft.begin(0x9341);
  tft.setRotation(45);
  tft.fillScreen(WHITE);
  display.drawMenu();
}

void loop() {
    static int menu = 0;
    static int scrollCounter = 0;
    static String currentBarcode = "";
    static String paymentStatus = "Unverified";
    static float previousWeightTotal = 0;
    static float currentWeightTotal = 0;
    static float tolerance = 0.2;
  
    // Check for RFID authorization
    if (menu == 5 && rfidManager.isAuthorized()) {
      paymentStatus = "Verified";
      display.drawVerifyCheckOut(cart.getTotalAmount(), paymentStatus);
      delay(1000);
      cart = ShoppingCart(); // Reset cart
      menu = 0;
      display.drawMenu();
    }
  
    // Check for barcode input from scanner
    if (Serial1.available()) {
      currentBarcode = Serial1.readStringUntil('\n');
      currentBarcode.trim(); // Remove extra spaces/newlines
      
      Serial.print("Scanned Barcode: ");
      Serial.println(currentBarcode);
  
      // Send barcode to ESP32
      Serial1.println(currentBarcode); // Use println for proper newline termination
  
      // Wait for ESP32 response
      unsigned long timeout = millis() + 500; // Max wait 500ms
      while (!Serial1.available() && millis() < timeout);
  
      // Read ESP32 response
      if (Serial1.available()) {
          String response = Serial1.readStringUntil('\n');
          response.trim();
      
          Serial.print("Received Data: ");
          Serial.println(response);
      
          //Parse JSON from ESP32
          StaticJsonDocument<200> doc;
          DeserializationError error = deserializeJson(doc, response);
          
          if (!error) {
              String name = doc["name"].as<String>();
              float price = doc["price"].as<float>();
              float weight = doc["weight"].as<float>();
      
              //Add item to the cart
              cart.addItem(currentBarcode, name, price, weight);
              display.drawProductList(cart, 0);
          } else {
              Serial.println("JSON Parse Error!");
          }
      } else {
          Serial.println("No response from ESP32!");
      }
    }
  }
  

  // Check for weight mismatch
  currentWeightTotal = scale.get_units(5);
  if (abs(currentWeightTotal - previousWeightTotal) > tolerance) {
    display.drawWeightMismatch();
    menu = 7;
  }

  // Handle touch screen input
  TSPoint p = ts.getPoint();
  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
    int x = map(p.y, TS_MINY, TS_MAXY, 0, tft.width());
    int y = map(p.x, TS_MINX, TS_MAXX, 0, tft.height());

    // Handle button presses
    if (x > 65 && x < 255 && y > 180 && y < 215) { // New Transaction button
      menu = 1;
      display.drawProductList(cart, scrollCounter);
    }
  }

  
}