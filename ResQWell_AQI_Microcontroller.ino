#include <TimerOne.h>
#include <SoftwareSerial.h>
#include <DHT.h>

/*
  ResQWell - AQI Microcontroller

  Hardware:
  DHT11       -> DHT_PIN (D2)
  HC-05       -> Arduino Serial 0(RX), 1(TX)
  ESP8266     -> SoftwareSerial D10(RX), D11(TX)
  AQI sensor  -> A1

  L298 Motor Driver:
    Motor 1 = Pulley UP/DOWN
    Motor 2 = Jaw OPEN/CLOSE

  Relay:
    Oxygen pump ON/OFF

  Bluetooth commands:
    U = Pulley UP
    D = Pulley DOWN
    O = Jaw OPEN
    C = Jaw CLOSE
    1 = Oxygen pump ON
    0 = Oxygen pump OFF
    S = Stop both motors

  Status is periodically sent to HC-05:
    AQI, Temperature, Humidity, Motor status, Oxygen pump status

  IMPORTANT:
  - Replace the Wi-Fi placeholders in INIT_WIFI() with your own credentials.
  - Do not commit real Wi-Fi passwords or other secrets to a public GitHub repo.
*/

#define DHT_PIN 2
#define DHT_TYPE DHT11

DHT dht(DHT_PIN, DHT_TYPE);

// ---------------- AQI ----------------
const int AQI_SNS_PIN = A1;

boolean AQI_Tenmec_Flg = 0;
int AQI_ADC = 0;
int AQI_ADC_VAL = 0;
int AQI_Cnt = 0;
int AQI_AVG = 0;

// ---------------- Wi-Fi / ESP8266 ----------------
SoftwareSerial mySerial(10, 11); // RX, TX

boolean UPDATE_DATA_WIFI = 0;
int WIFI_CNT = 0;

// ---------------- Timer ----------------
volatile int msec_cnt = 0;
volatile int MS_CNT1 = 0;
volatile int TEN_MCNT = 0;

// ---------------- DHT11 ----------------
float TEMPERATURE = 0;
float HUMIDITY = 0;

unsigned long lastDHTRead = 0;
const unsigned long DHT_INTERVAL = 2000;

// ---------------- Bluetooth status ----------------
unsigned long lastBTStatus = 0;
const unsigned long BT_STATUS_INTERVAL = 2000;

// ---------------- L298 Motor Driver ----------------
// Motor 1: Pulley
const int PULLEY_IN1 = 4;
const int PULLEY_IN2 = 5;

// Motor 2: Jaw
const int JAW_IN1 = 6;
const int JAW_IN2 = 7;

// ---------------- Oxygen relay ----------------
const int OXYGEN_RELAY_PIN = 8;

// Set to LOW if your relay module is active LOW.
const int RELAY_ON = HIGH;
const int RELAY_OFF = LOW;

// ---------------- Motor status ----------------
String pulleyStatus = "STOP";
String jawStatus = "STOP";
bool oxygenPumpStatus = false;


// ========================================================
// SETUP
// ========================================================
void setup()
{
  // HC-05 is connected to hardware Serial 0,1
  Serial.begin(9600);

  // ESP8266 remains on SoftwareSerial
  mySerial.begin(115200);

  dht.begin();

  pinMode(AQI_SNS_PIN, INPUT);

  pinMode(PULLEY_IN1, OUTPUT);
  pinMode(PULLEY_IN2, OUTPUT);

  pinMode(JAW_IN1, OUTPUT);
  pinMode(JAW_IN2, OUTPUT);

  pinMode(OXYGEN_RELAY_PIN, OUTPUT);

  stopPulley();
  stopJaw();
  oxygenOff();

  // Timer interrupt every 1 ms
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);

  // ESP8266 initialization
  mySerial.println("AT");
  delay(300);
  INIT_WIFI();

  // Initial sensor read
  readDHT11();

  Serial.println("SYSTEM READY");
  Serial.println("Bluetooth Commands: U=UP D=DOWN O=OPEN C=CLOSE 1=OXYGEN ON 0=OXYGEN OFF S=STOP");
}


// ========================================================
// MAIN LOOP
// ========================================================
void loop()
{
  READ_AQI_SENSOR();

  readDHT11();

  readBluetoothCommand();

  if (UPDATE_DATA_WIFI)
  {
    esp_8266();
    UPDATE_DATA_WIFI = 0;
  }

  sendBluetoothStatus();
}


// ========================================================
// TIMER ISR
// ========================================================
void timerIsr()
{
  // 500 ms counter
  msec_cnt++;

  if (msec_cnt >= 500)
  {
    if (UPDATE_DATA_WIFI == 0)
    {
      WIFI_CNT++;

      // 20 x 500 ms = 10 seconds
      if (WIFI_CNT >= 20)
      {
        UPDATE_DATA_WIFI = 1;
        WIFI_CNT = 0;
      }
    }

    msec_cnt = 0;
  }

  // 1 second counter
  MS_CNT1++;

  if (MS_CNT1 >= 1000)
  {
    MS_CNT1 = 0;
  }

  // AQI sampling every 10 ms
  TEN_MCNT++;

  if (TEN_MCNT >= 10)
  {
    AQI_Tenmec_Flg = 1;
    TEN_MCNT = 0;
  }
}


// ========================================================
// AQI SENSOR
// ========================================================
void READ_AQI_SENSOR(void)
{
  if (AQI_Tenmec_Flg)
  {
    AQI_ADC = analogRead(AQI_SNS_PIN);
    AQI_ADC_VAL = AQI_ADC_VAL + AQI_ADC;
    AQI_Cnt++;
    AQI_Tenmec_Flg = 0;
  }

  if (AQI_Cnt >= 8)
  {
    AQI_AVG = AQI_ADC_VAL >> 3;

    AQI_Cnt = 0;
    AQI_ADC_VAL = 0;
  }
}


// ========================================================
// DHT11
// ========================================================
void readDHT11()
{
  if (millis() - lastDHTRead >= DHT_INTERVAL)
  {
    lastDHTRead = millis();

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (!isnan(h) && !isnan(t))
    {
      HUMIDITY = h;
      TEMPERATURE = t;
    }
  }
}


// ========================================================
// BLUETOOTH COMMAND READING
// HC-05 -> Serial RX pin 0
// ========================================================
void readBluetoothCommand()
{
  while (Serial.available() > 0)
  {
    char command = Serial.read();

    // Ignore CR/LF and spaces
    if (command == '\r' || command == '\n' || command == ' ')
      continue;

    switch (command)
    {
      case 'U':
      case 'u':
        pulleyUp();
        break;

      case 'D':
      case 'd':
        pulleyDown();
        break;

      case 'O':
      case 'o':
        jawOpen();
        break;

      case 'C':
      case 'c':
        jawClose();
        break;

      case '1':
        oxygenOn();
        break;

      case '0':
        oxygenOff();
        break;

      case 'S':
      case 's':
        stopPulley();
        stopJaw();
        break;

      default:
        Serial.println("INVALID COMMAND");
        break;
    }
  }
}


// ========================================================
// PULLEY MOTOR - MOTOR 1
// ========================================================
void pulleyUp()
{
  digitalWrite(PULLEY_IN1, HIGH);
  digitalWrite(PULLEY_IN2, LOW);
  pulleyStatus = "UP";
}

void pulleyDown()
{
  digitalWrite(PULLEY_IN1, LOW);
  digitalWrite(PULLEY_IN2, HIGH);
  pulleyStatus = "DOWN";
}

void stopPulley()
{
  digitalWrite(PULLEY_IN1, LOW);
  digitalWrite(PULLEY_IN2, LOW);
  pulleyStatus = "STOP";
}


// ========================================================
// JAW MOTOR - MOTOR 2
// ========================================================
void jawOpen()
{
  digitalWrite(JAW_IN1, HIGH);
  digitalWrite(JAW_IN2, LOW);
  jawStatus = "OPEN";
}

void jawClose()
{
  digitalWrite(JAW_IN1, LOW);
  digitalWrite(JAW_IN2, HIGH);
  jawStatus = "CLOSE";
}

void stopJaw()
{
  digitalWrite(JAW_IN1, LOW);
  digitalWrite(JAW_IN2, LOW);
  jawStatus = "STOP";
}


// ========================================================
// OXYGEN PUMP RELAY
// ========================================================
void oxygenOn()
{
  digitalWrite(OXYGEN_RELAY_PIN, RELAY_ON);
  oxygenPumpStatus = true;
}

void oxygenOff()
{
  digitalWrite(OXYGEN_RELAY_PIN, RELAY_OFF);
  oxygenPumpStatus = false;
}


// ========================================================
// SEND STATUS TO HC-05
// ========================================================
void sendBluetoothStatus()
{
  if (millis() - lastBTStatus >= BT_STATUS_INTERVAL)
  {
    lastBTStatus = millis();

    Serial.print("AQI:");
    Serial.print(AQI_AVG);

    Serial.print(",TEMP:");
    Serial.print(TEMPERATURE, 1);

    Serial.print(",HUM:");
    Serial.print(HUMIDITY, 1);

    Serial.print(",PULLEY:");
    Serial.print(pulleyStatus);

    Serial.print(",JAW:");
    Serial.print(jawStatus);

    Serial.print(",OXYGEN:");
    Serial.println(oxygenPumpStatus ? "ON" : "OFF");
  }
}


// ========================================================
// ESP8266 / WI-FI
// ========================================================
void esp_8266(void)
{
  String cmd = "AT+CIPSTART=4,\"TCP\",\"";
  cmd += "172.105.39.109";
  cmd += "\",8091";

  mySerial.println(cmd);
  delay(500);

  if (mySerial.find("Error"))
  {
    mySerial.println("AT+CIPSTART error");
    return;
  }

  // HTTP request sends temperature, humidity and AQI to the server.
  String getStr = "GET /AQI/data/send?";

  getStr += "temperature=";
  getStr += String(TEMPERATURE, 1);

  getStr += "&humidity=";
  getStr += String(HUMIDITY, 1);

  getStr += "&aqi=";
  getStr += String(AQI_AVG);

  getStr += "\r\n\r\n";

  cmd = "AT+CIPSEND=4,";
  cmd += String(getStr.length());

  mySerial.println(cmd);
  delay(1000);

  mySerial.println(getStr);

  delay(1000);
  mySerial.println("AT+CIPCLOSE=4");
}


// ========================================================
// ESP8266 INITIALIZATION
// ========================================================
void INIT_WIFI(void)
{
  mySerial.println("AT+CWMODE=3");
  delay(1000);

  mySerial.println("AT+CWQAP");
  delay(1000);

  mySerial.println("AT+CIPMUX=1");
  delay(1000);

  // Enter your Wi-Fi credentials locally. Do not publish them on GitHub.
  String cmd = "AT+CWJAP=\"YOUR_WIFI_NAME\",\"YOUR_WIFI_PASSWORD\"";
  mySerial.println(cmd);

  delay(6000);
}
