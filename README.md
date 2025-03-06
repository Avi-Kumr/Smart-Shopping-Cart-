# 📦 Smart Shopping Cart (Self-Checkout System)

This project is a **Smart Shopping Cart** with an **RFID-based self-checkout system**, integrating:  
✅ **Arduino Mega** (for GUI, barcode scanning, weight detection)  
✅ **ESP32** (for fetching product details from a local server)  
✅ **HX711** (for weight measurement)  
✅ **MFRC522** (for RFID authentication)  
✅ **TFT Touch Display** (for user interaction)  

## 📌 Features  
- **Barcode Scanning:** Scan items and retrieve product details from the server  
- **Weight Detection:** Compares actual and expected weight for security  
- **RFID Checkout:** Users must verify via RFID before completing checkout  
- **Touchscreen UI:** Displays items, total price, and guides users through checkout  
- **ESP32 Communication:** Retrieves product data via HTTP requests  

---

## 🛠️ Hardware Requirements  
- **Arduino Mega 2560**  
- **ESP32** (Wi-Fi module)  
- **TFT Touchscreen (ILI9341)**  
- **HX711 + Load Cell** (for weight measurement)  
- **RFID Module (MFRC522)**  
- **Barcode Scanner**  

---
