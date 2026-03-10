#include <ArduinoWebsockets.h>
#include "esp_http_server.h"
#include "esp_timer.h"
#include "esp_camera.h"
#include "camera_index.h"
#include "Arduino.h"
#include "fd_forward.h"
#include "fr_forward.h"
#include "fr_flash.h"
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP_Mail_Client.h>

const char* ssid = "Mywifi";
const char* password = "1234567890";

#define ENROLL_CONFIRM_TIMES 5
#define FACE_ID_SAVE_NUMBER 7

// Gmail configuration
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465
const char* smtp_email = "your_email@gmail.com";  // Sender email
const char* smtp_password = "your_email_password";
const char* recipient_email = "owner_email@gmail.com"; // Owner email

// LCD Display Setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Select camera model
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

using namespace websockets;
WebsocketsServer socket_server;

camera_fb_t* fb = NULL;

long current_millis;
long last_detected_millis = 0;

#define relay_pin 2
unsigned long door_opened_millis = 0;
long interval = 5000;
bool face_recognised = false;

void app_facenet_main();
void app_httpserver_init();
void send_email_notification(const char* subject, const char* message, camera_fb_t* frame);

typedef struct
{
    uint8_t* image;
    box_array_t* net_boxes;
    dl_matrix3d_t* face_id;
} http_img_process_result;

// Face detection and recognition configuration
static inline mtmn_config_t app_mtmn_config()
{
    mtmn_config_t mtmn_config = {0};
    mtmn_config.type = FAST;
    mtmn_config.min_face = 80;
    mtmn_config.pyramid = 0.707;
    mtmn_config.pyramid_times = 4;
    mtmn_config.p_threshold.score = 0.6;
    mtmn_config.p_threshold.nms = 0.7;
    mtmn_config.p_threshold.candidate_number = 20;
    mtmn_config.r_threshold.score = 0.7;
    mtmn_config.r_threshold.nms = 0.7;
    mtmn_config.r_threshold.candidate_number = 10;
    mtmn_config.o_threshold.score = 0.7;
    mtmn_config.o_threshold.nms = 0.7;
    mtmn_config.o_threshold.candidate_number = 1;
    return mtmn_config;
}
mtmn_config_t mtmn_config = app_mtmn_config();

face_id_name_list st_face_list;
static dl_matrix3du_t* aligned_face = NULL;
httpd_handle_t camera_httpd = NULL;

typedef enum
{
    START_STREAM,
    START_DETECT,
    SHOW_FACES,
    START_RECOGNITION,
    START_ENROLL,
    ENROLL_COMPLETE,
    DELETE_ALL,
} en_fsm_state;
en_fsm_state g_state;

typedef struct
{
    char enroll_name[ENROLL_NAME_LEN];
} httpd_resp_value;

httpd_resp_value st_name;

void setup()
{
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Serial.println();

    // LCD Initialization
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Initializing...");

    digitalWrite(relay_pin, LOW);
    pinMode(relay_pin, OUTPUT);

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    if (psramFound()) {
        config.frame_size = FRAMESIZE_UXGA;
        config.jpeg_quality = 10;
        config.fb_count = 2;
    } else {
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }

    // Camera initialization
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        lcd.setCursor(0, 1);
        lcd.print("Cam Init Failed");
        return;
    }

    sensor_t* s = esp_camera_sensor_get();
    s->set_framesize(s, FRAMESIZE_QVGA);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    lcd.setCursor(0, 1);
    lcd.print("WiFi Connected");

    app_httpserver_init();
    app_facenet_main();
    socket_server.listen(82);

    Serial.print("Camera Ready! Use 'http://");
    Serial.print(WiFi.localIP());
    Serial.println("' to connect");
}

void send_email_notification(const char* subject, const char* message, camera_fb_t* frame)
{
    SMTPSession smtp;
    smtp.debug(1);

    ESP_Mail_Session session;
    session.server.host_name = SMTP_HOST;
    session.server.port = SMTP_PORT;
    session.login.email = smtp_email;
    session.login.password = smtp_password;
    session.login.user_domain = "";

    SMTP_Message email;
    email.sender.name = "ESP32 Face Recog";
    email.sender.email = smtp_email;
    email.subject = subject;
    email.addRecipient("Owner", recipient_email);

    email.text.content = message;
    email.text.charSet = "us-ascii";
    email.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

    // Attach face image
    email.addInlineImage("face.jpg", frame->buf, frame->len, "image/jpeg");

    if (!MailClient.sendMail(&smtp, &session, &email)) {
        Serial.printf("Error sending Email, %s\n", smtp.errorReason().c_str());
    }
    smtp.closeSession();
}

void loop()
{
    auto client = socket_server.accept();
    client.onMessage(handle_message);

    while (client.available()) {
        client.poll();

        fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("Camera capture failed");
            return;
        }

        if (g_state == START_DETECT) {
            lcd.setCursor(0, 1);
            lcd.print("Detecting Face");

            // Simulated unrecognized face detection
            send_email_notification("Unrecognized Face Detected", "An unrecognized face has been detected.", fb);
            lcd.setCursor(0, 1);
            lcd.print("Email Sent");
        }

        esp_camera_fb_return(fb);
    }
}
