# ResQWell – AQI Microcontroller

Arduino firmware for the ResQWell prototype. It reads AQI, temperature and humidity, controls the pulley and robotic gripper jaw motors, switches the oxygen-pump relay, communicates with an HC-05 Bluetooth module, and sends sensor readings through an ESP8266 Wi-Fi module.

## Folder contents

- `ResQWell_AQI_Microcontroller.ino` – Arduino source code.

## Hardware connections

| Component | Arduino connection |
|---|---|
| DHT11 data | D2 |
| AQI sensor analog output | A1 |
| HC-05 | Hardware Serial RX/TX (D0/D1) |
| ESP8266 | SoftwareSerial RX D10, TX D11 |
| L298 motor driver – pulley | IN1 D4, IN2 D5 |
| L298 motor driver – jaw | IN1 D6, IN2 D7 |
| Oxygen-pump relay | D8 |

**Important:** Check the voltage requirements of each module. Use a suitable level shifter for Arduino-to-ESP8266 signals where required, and use an appropriate external motor supply. Do not power motors directly from an Arduino I/O pin.

## Bluetooth commands

| Command | Action |
|---|---|
| `U` | Pulley up |
| `D` | Pulley down |
| `O` | Robotic gripper jaw open |
| `C` | Robotic gripper jaw close |
| `1` | Oxygen pump ON |
| `0` | Oxygen pump OFF |
| `S` | Stop both motors |

The firmware sends AQI, temperature, humidity, pulley status, jaw status and oxygen-pump status to the HC-05 approximately every two seconds.

## Arduino IDE setup

1. Install the Arduino IDE.
2. Install the **TimerOne** and **DHT sensor library** libraries from Library Manager.
3. Open `ResQWell_AQI_Microcontroller.ino`.
4. Select the correct Arduino board and COM port.
5. Add your Wi-Fi name and password in `INIT_WIFI()` on your local copy.
6. Upload the sketch and test each sensor and actuator safely.

## Wi-Fi configuration and GitHub safety

The Wi-Fi credentials in the source are placeholders. Replace `YOUR_WIFI_NAME` and `YOUR_WIFI_PASSWORD` only in your local working copy. **Never commit real Wi-Fi credentials, API keys, or passwords to a public repository.** If credentials were already exposed in a public repository, change the Wi-Fi password.

The ESP8266 code connects to the configured server at `172.105.39.109:8091` and requests `/AQI/data/send` with temperature, humidity and AQI query parameters. Update the server address and port if your backend changes.

## Notes

- HC-05 uses Arduino hardware serial pins D0/D1. Disconnect the HC-05 while uploading if it interferes with serial uploading.
- Confirm whether your relay module is active-HIGH or active-LOW. Adjust `RELAY_ON` and `RELAY_OFF` accordingly.
- Test the motor direction and mechanical limits without a person attached to the mechanism. Add suitable limit switches and emergency-stop protection before any real rescue-system testing.
- This is prototype firmware; sensor calibration and end-to-end safety validation are required before field use.
