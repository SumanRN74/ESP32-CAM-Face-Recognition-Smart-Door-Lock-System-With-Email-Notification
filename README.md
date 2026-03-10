# ESP32-CAM-Face-Recognition-Smart-Door-Lock-System-With-Email-Notification

An **AI-based smart access control system** built using **ESP32-CAM** that performs **face detection and recognition** to control a door lock.

The system automatically unlocks the door for recognized faces. If an **unknown face appears**, it **sends an email notification with the captured image to the owner**, allowing them to **remotely allow or deny access**.

The system also integrates an **MC38 magnetic door sensor** to monitor door status and display it on an **LCD display**.

---

# System Overview

This project implements a **smart security door lock system** using **ESP32-CAM and face recognition technology**.

The system provides:

- Automatic access for authorized users
- Security alert for unknown persons
- Remote door control via email
- Door status monitoring
- LCD status display
- Live camera streaming through a web interface

---

# System Architecture

```
                Person Approaches Door
                        │
                        ▼
                  ESP32-CAM Camera
                  (Image Capture)
                        │
                        ▼
                 Face Detection
                 (MTCNN Algorithm)
                        │
                        ▼
                 Face Recognition
                 (FaceNet Model)
                        │
                        ▼
                  Decision System
                        │
        ┌───────────────┴───────────────┐
        │                               │
        ▼                               ▼
   Face Recognized                 Face Not Recognized
        │                               │
        ▼                               ▼
   Activate Relay                  Capture Image
   Unlock Door                     Send Email Alert
   Display Name on LCD             Owner Receives Email
        │                               │
        ▼                               ▼
   Door Opens                   Owner Selects Action
                                    │
                         ┌──────────┴──────────┐
                         │                     │
                         ▼                     ▼
                    Allow Access           Deny Access
                         │                     │
                         ▼                     ▼
                    Unlock Door             Keep Locked
```

---

# Hardware Components

| Component | Quantity |
|----------|----------|
| ESP32-CAM | 1 |
| FTDI Programmer | 1 |
| Relay Module | 1 |
| Solenoid Door Lock | 1 |
| MC38 Magnetic Door Sensor | 1 |
| 16x2 I2C LCD Display | 1 |
| 5V Power Supply | 1 |
| Jumper Wires | As required |

---

# Pin Connections

## Relay Lock

| ESP32 Pin | Component |
|----------|-----------|
| GPIO 2 | Relay IN |
| 5V | Relay VCC |
| GND | Relay GND |

---

## Door Sensor (MC38)

| ESP32 Pin | Sensor |
|----------|--------|
| GPIO 12 | Sensor Output |
| GND | Sensor GND |

---

## LCD Display (I2C)

| ESP32 Pin | LCD |
|----------|-----|
| GPIO 14 | SDA |
| GPIO 15 | SCL |

---

# Software Requirements

Install the following libraries in **Arduino IDE**.

## ESP32 Board Package

Add this URL in Arduino IDE:

```
https://dl.espressif.com/dl/package_esp32_index.json
```

---

# Required Libraries

Install these libraries:

- ArduinoWebsockets
- ESP Mail Client
- LiquidCrystal_I2C
- ESP32 Camera Driver

---

# WiFi Configuration

Update WiFi credentials in the code.

```cpp
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
```

---

# Email Configuration

Configure Gmail SMTP.

```cpp
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

#define AUTHOR_EMAIL "your_email@gmail.com"
#define AUTHOR_PASSWORD "your_app_password"

#define RECIPIENT_EMAIL "owner_email@gmail.com"
```

Use a **Google App Password**, not your normal Gmail password.

---

# Face Enrollment

To add a new user:

1. Open the ESP32 web interface
2. Click **Capture**
3. Enter the person's name
4. Capture **5 face samples**

The system stores the face data in **ESP32 flash memory**.

---

# Maximum Supported Faces

Default system supports:

```
7 Faces
```

You can increase this by modifying:

```cpp
FACE_ID_SAVE_NUMBER
```

---

# Web Interface

Open the system in a browser using:

```
http://ESP32_IP_ADDRESS
```

The web interface allows:

- Camera streaming
- Face detection
- Face enrollment
- Face recognition
- Delete saved faces

---

# Security Workflow

```
Person approaches door
        │
        ▼
Camera detects face
        │
        ▼
Face matched with database?
        │
   ┌────┴─────┐
   │          │
  YES        NO
   │          │
   ▼          ▼
Open Door   Send Email Alert
Show Name   Wait for Owner Decision
```

---

# Applications

This system can be used in:

- Smart homes
- Offices
- Secure laboratories
- Hostels
- IoT security systems

---

# Future Improvements

Possible upgrades:

- Mobile app control
- Cloud face database
- Firebase logging
- Motion detection integration
- Multi-camera security system

---

# License

This project is open-source and free to use for educational and research purposes.


## 👨‍💻 Author
**Suman R N**  
📧 Contact: sumansurn@gmail.com 
🔗 LinkedIn: https://www.linkedin.com/in/suman-r-1b5260335 

