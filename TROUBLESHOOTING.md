# HC-LEP Troubleshooting Guide

## Fixed Issues

### 1. ✅ OLED Display "ENO" Error - FIXED
**Problem:** Display showing error messages continuously  
**Cause:** OLED not properly initialized or I2C connection issue  
**Solution Applied:**
- Added `displayOK` flag to check display status
- Device now continues working even if display fails
- All display updates are now conditional

### 2. ✅ Data Not Sending to Server - ENHANCED
**Problem:** ESP32 not successfully transmitting data to web server  
**Causes & Solutions:**

#### Enhanced Debugging
- Added detailed Serial output for every step
- Added timeout handling for server responses (5 seconds)
- Clear error messages for connection failures

#### What to Check Now

**A. Open Serial Monitor**
1. In Arduino IDE, go to **Tools → Serial Monitor**
2. Set baud rate to **115200**
3. Watch the debug messages:
   ```
   HC-LEP Starting...
   Display OK!
   Connecting to WiFi: SURESHKUMAR
   WiFi Connected!
   IP: 192.168.x.x
   Distance measured: 25 cm
   Connecting to server: 43.204.238.139:8080
   Connected to server!
   Received public key: xxx
   Generated session key: xx
   Plaintext: D:0025
   Ciphertext (hex): xxxxx
   Data sent to server!
   ```

**B. Common Error Messages & Fixes**

| Error Message | Cause | Solution |
|--------------|--------|----------|
| `SSD1306 allocation failed` | Display not connected | Check I2C wiring (SDA=D21, SCL=D22) or continue without display |
| `WiFi Connection Failed!` | Wrong credentials or router issue | Verify WiFi name/password in code |
| `Failed to connect to server!` | Server not running/wrong IP | Check server is running on port 8080 |
| `Timeout waiting for server response!` | Server not responding | Check server.py is running properly |

**C. Server Connection Checklist**

1. **Is the server running?**
   ```bash
   python server.py
   ```
   Should show:
   ```
   HC-LEP Server listening on 8080...
   * Running on http://0.0.0.0:5000
   ```

2. **Is the server IP correct?**
   - Check AWS EC2 public IP: `43.204.238.139`
   - Make sure ESP32 has this IP in code (line 18)

3. **Are ports open?**
   - Server needs port 8080 (encryption server)
   - Server needs port 5000 (web dashboard)
   - Check AWS Security Group rules

4. **Network connectivity?**
   - ESP32 and server must be able to reach each other
   - Try pinging server from ESP32's network

**D. Testing Locally First**

To test on your local network before using AWS:

1. **Find your computer's local IP:**
   ```bash
   ipconfig  # Windows
   # Look for IPv4 Address (e.g., 192.168.1.100)
   ```

2. **Update ESP32 code:**
   ```cpp
   const char* serverIP = "192.168.1.100";  // Your computer's IP
   ```

3. **Run server locally:**
   ```bash
   python server.py
   ```

4. **Upload and test ESP32**
   - Watch Serial Monitor for connection messages
   - Check server terminal for incoming connections

## Hardware Connections

### OLED Display (SSD1306)
- **VCC** → 3.3V
- **GND** → GND
- **SDA** → GPIO 21 (D21)
- **SCL** → GPIO 22 (D22)

### Ultrasonic Sensor (HC-SR04)
- **VCC** → 5V (VIN)
- **GND** → GND
- **TRIG** → GPIO 5 (D5)
- **ECHO** → GPIO 18 (D18)

## Web Dashboard

Access at: `http://43.204.238.139:5000`

If showing "OFFLINE" warning:
1. Check ESP32 Serial Monitor - is it sending data?
2. Check server.py terminal - is it receiving data?
3. Wait 5 seconds for watchdog to clear if just started

## Need More Help?

1. **Copy Serial Monitor output** and check for specific errors
2. **Check server.py terminal** for connection logs
3. **Verify hardware connections** with multimeter if needed
4. **Try local network test** before using AWS
