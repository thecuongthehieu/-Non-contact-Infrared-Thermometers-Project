#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>


/* FOR ESP8266 */
#define CMD_SEND_BEGIN  "AT+CIPSEND=0"
#define CMD_SEND_END    "AT+CIPCLOSE=0"
 
#define PROTOCOL_HTTP     80
#define PROTOCOL_HTTPS    443
#define PROTOCOL_FTP      21
#define PROTOCOL_CURRENT  PROTOCOL_HTTP
 
#define CHAR_CR     0x0D
#define CHAR_LF     0x0A
 
#define DELAY_SEED  2000
#define DELAY_1X    (1 * DELAY_SEED)
#define DELAY_2X    (2 * DELAY_SEED)
#define DELAY_3X    (3 * DELAY_SEED)
#define DELAY_4X    (4 * DELAY_SEED)
#define DELAY_5X    (5 * DELAY_SEED)

/* WIFI CONNECTION INF*/
String WIFI_NAME = "\"Cuong Hieu\"";
String WIFI_PASSWORD = "\"phanboichau\"";

/* SERVER CONNECTION*/
String HOST = "\"f4e695bc.ngrok.io\"";
String HOST_STRING = "f4e695bc.ngrok.io ";
String PORT = "80";



/* FOR KEYPAD AND LCD */
const byte ROWS = 4;
const byte COLS = 4;

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

LiquidCrystal_I2C lcd(0x27, 16, 2);  


/* FOR DISTANCE */ 

// defines pins numbers
const int trigPin = 11;
const int echoPin = 10;

// defines variables
long duration;
int distance;


/* FOR THERMOMETER */
int16_t accX, accY, accZ;


void setup()
{
  delay(DELAY_5X);

  //ESP8266 set up
  Serial.begin(115200);
  initESP8266();

  //LCD set up
  lcd.backlight();
  lcd.init(); 
  lcd.clear();

  //Distance set up
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input

  //Thermometer set up
  Wire.begin();
  Wire.beginTransmission(0x68); // Begins a transmission to the I2C slave (GY-521 board)
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);

  Serial.println("START");
  delay(5000);
}


void loop()
{
  lcd.setCursor(2,0);
  lcd.print("No One!");
  
  updateDistance();
  Serial.println(distance);
  
  if (distance < 5) {
    
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("Hi guys!");
    
    readAcc(); // get Temp

    while (distance < 5) {
      updateDistance();
      delay(1000);
    }

    Serial.println(accZ);
    
    displayTemperature();
    
    if (accZ < 0) {
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("Type MSSV: ");

      int pos = 1;
      String MSSV = "";
      
      while (true) {
        char customKey = customKeypad.getKey();
        if (customKey){
          if (customKey == '*') {
            break;
          }

          MSSV += customKey;
          lcd.setCursor(pos, 1); 
          lcd.print(customKey);
          pos += 1;
        }
      }

      lcd.clear();
      lcd.setCursor(2,0); 
      lcd.print("Sending...");
      sendToServer(MSSV);
      lcd.clear();
      lcd.setCursor(2,0); 
      lcd.print("Be Sent");
    } 

    delay(2000);
    lcd.clear();
  } 
    
}

void initESP8266()
{
  /* MODE SELECTION */
  deliverMessage("AT+CWMODE=1", DELAY_2X);
  deliverMessage("AT+CIPMUX=0", DELAY_2X); 
  
  /* WIFI CONNECTION */
  deliverMessage("AT+CWJAP=" + WIFI_NAME + "," + WIFI_PASSWORD, DELAY_5X); 
}

void updateValueGETAPI(String value) {
  /* HOST CONNECTION COMMAND */
  deliverMessage("AT+CIPSTART=\"TCP\"," + HOST + "," + PORT, DELAY_4X);

  /* GET API SENDING */
  String mess1 =  "GET /update/" + value + " HTTP/1.1";
  String mess2 = "Host: " + HOST_STRING;
  String mess3 = "Connection: keep-alive";
  int mess_len = mess1.length() + mess2.length() + mess3.length() + 8;

  deliverMessage("AT+CIPSEND=" + String(mess_len), DELAY_2X);
  deliverMessage(mess1, 0);
  deliverMessage(mess2, 0);
  deliverMessage(mess3, 0);
  Serial.println();
}

void sendToServer(String value) {
  updateValueGETAPI(value);
  delay(DELAY_4X);
}


void deliverMessage(const String& msg, int dt)
{
  Serial.println(msg);
  delay(dt);
}

void updateDistance() {
   // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2;
  // Prints the distance on the Serial Monitor
}

void displayTemperature() {
  lcd.setCursor(0, 0); // Set the cursor on the first column and first row.
  lcd.print("Temp: "); // Print the string "Hello World!"
  lcd.setCursor(6, 0);
  lcd.print(String(accZ));

  lcd.setCursor(2, 1);
  if (accZ >= 0) {
    lcd.print("Well");
  } else {
    lcd.print("Seriously");
  }

  delay(5000);
}


void readAcc() {
  Wire.beginTransmission(0x68);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H) [MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2, p.40]
  Wire.endTransmission(false); // the parameter indicates that the Arduino will send a restart. As a result, the connection is kept active.
  Wire.requestFrom(0x68, 3 * 2, true); // request a total of 7*2=14 registers
  
  // "Wire.read()<<8 | Wire.read();" means two registers are read and stored in the same variable
  accX = Wire.read()<<8 | Wire.read(); // reading registers: 0x3B (ACCEL_XOUT_H) and 0x3C (ACCEL_XOUT_L)
  accY = Wire.read()<<8 | Wire.read(); // reading registers: 0x3D (ACCEL_YOUT_H) and 0x3E (ACCEL_YOUT_L)
  accZ = Wire.read()<<8 | Wire.read(); // reading registers: 0x3F (ACCEL_ZOUT_H) and 0x40 (ACCEL_ZOUT_L)
}
