#include <WiFi.h>
#include "mbedtls/aes.h"
#include <math.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define TRIG 5
#define ECHO 18
#define SERVO_PIN 13

Servo myServo;
int angle = 0;
int direction = 1;

const char* ssid = "SURESHKUMAR";
const char* password = "SURESHKUMAR";
const char* serverIP = "3.110.86.38"; // AWS EC2 Instance

WiFiClient client;
bool displayOK = false;

unsigned long chaos_state = 123456789;

const int N = 4;
const int Q = 255;
uint8_t pq_key[16]; 
uint8_t plain[16];
uint8_t cipher[16];

float chaos_iterate() {
    chaos_state = (chaos_state * 1664525 + 1013904223); 
    return (float)chaos_state / 4294967296.0;
}

void aesEncrypt(uint8_t *input, uint8_t *output) {
    mbedtls_aes_context ctx;
    mbedtls_aes_init(&ctx);
    mbedtls_aes_setkey_enc(&ctx, pq_key, 128);
    mbedtls_aes_crypt_ecb(&ctx, MBEDTLS_AES_ENCRYPT, input, output);
    mbedtls_aes_free(&ctx);
}

int getDistance() {
    digitalWrite(TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG, LOW);
    long d = pulseIn(ECHO, HIGH);
    float dist = d * 0.034 / 2;
    if (dist > 200) dist = 200; // Cap distance
    return (int)dist;
}

void updateDisplay(String status, int dist, int ang, uint8_t* cipherArr) {
    if(!displayOK) return; 
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    // Header
    display.setCursor(0,0);
    display.println(">> HC-LEP SECURE NODE");
    display.drawLine(0, 9, 128, 9, SSD1306_WHITE);
    
    // Sensing Row
    display.setCursor(0, 14);
    display.print("SENSE: "); 
    display.print(dist); display.print("cm @ "); 
    display.print(ang); display.println("deg");
    
    // Encryption Row
    display.setCursor(0, 26);
    display.print("MODE:  AES-128 CHAOS");
    
    display.setCursor(0, 36);
    display.print("CIPH:  ");
    if (cipherArr) {
        for(int i=0; i<6; i++) {
            if(cipherArr[i] < 0x10) display.print("0");
            display.print(cipherArr[i], HEX);
        }
        display.print("...");
    } else {
        display.print("WAITING...");
    }
    
    // Status Row
    display.drawLine(0, 50, 128, 50, SSD1306_WHITE);
    display.setCursor(0, 55);
    display.print("STATUS: "); 
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Inverted text for status
    display.print(" " + status + " ");
    display.setTextColor(SSD1306_WHITE);
    
    display.display();
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("HC-LEP Starting...");
    
    myServo.attach(SERVO_PIN);
    
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
        Serial.println(F("SSD1306 allocation failed"));
        displayOK = false;
    } else {
        Serial.println("Display OK!");
        displayOK = true;
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0,20);
        display.println("Booting HC-LEP...");
        display.display();
    }
    
    pinMode(TRIG, OUTPUT);
    pinMode(ECHO, INPUT);

    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 40) {
        delay(500);
        Serial.print(".");
        if(displayOK) {
            display.print("."); 
            display.display();
        }
        attempts++;
    }
    
    if(WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi Connected!");
        if(displayOK) updateDisplay("READY", 0, 0, NULL);
    } else {
        Serial.println("\nWiFi Connection Failed!");
        if(displayOK) updateDisplay("WIFI FAIL", 0, 0, NULL);
    }
}

void loop() {
    // Sweep logic
    myServo.write(angle);
    delay(30); // Allow servo to move
    
    int distance = getDistance();
    Serial.print("Dist: "); Serial.print(distance); 
    Serial.print(" | Angle: "); Serial.println(angle);
    
    if(displayOK) updateDisplay("ENCRYPTING", distance, angle, NULL);
    
    if (client.connect(serverIP, 8080)) {
        chaos_state = 123456789;
        
        unsigned long startTime = millis();
        while(!client.available() && (millis() - startTime < 2000)) delay(10);
        
        if(client.available()) {
            String pk = client.readStringUntil('\n');
            chaos_iterate(); 
            int secret = (chaos_state >> 24) & 0xFF;
            memset(pq_key, secret, 16);
            
            memset(plain, 0, 16);
            // New format: D:XXX,A:YYY
            snprintf((char*)plain, 16, "D:%03d,A:%03d", distance, angle);
            
            aesEncrypt(plain, cipher);
            if(displayOK) updateDisplay("SENDING...", distance, angle, cipher);
            
            client.write(cipher, 16);
            client.stop();
            if(displayOK) updateDisplay("TX SUCCESS", distance, angle, cipher);
        }
    } else {
        if(displayOK) updateDisplay("CONN FAIL", distance, angle, NULL);
    }
    
    // Update angle
    angle += (direction * 5);
    if(angle >= 180 || angle <= 0) {
        direction *= -1;
    }
    delay(10);
}