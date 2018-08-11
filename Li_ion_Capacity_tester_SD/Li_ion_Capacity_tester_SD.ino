/** SD card attached to Arduino Nano as follows:
 **   ICSP Header
 **      o o
 ** MOSI O O SCK
 **      o O MOSI
 **
 ** CS - pin 4
 **/
#include <U8g2lib.h>
#include <SPI.h>
#include <SD.h>
U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // All Boards without Reset of the Display

#define chipSelect 4    //CS Pin on SD module
#define MOSFET_Pin 3    //Mosfet Gate pin
#define Buzz_Pin 9      //Buzzer pin
#define Vsense_P A0     //Positive Voltage Sense
#define Vsense_N A1     //Negative Voltage Sense

float Res_Value = 10.0; //Value of Power Resistor used
float Vcc = 4.70;       //Arduino 5V pin Voltage (Mesured by Multimeter)
float Bat_High = 4.35;  //Battery Over Voltage
float Bat_Low = 2.90;   //Battery Under Voltage
float Current = 0.00;
float mA = 0;
float Capacity = 0.00;
float Bat_Volt = 0.00;
float Res_Volt = 0.00;
unsigned long previousMillis = 0;
unsigned long millisPassed = 0;
float sample1 = 0.000;
float sample2 = 0.000;
int x = 0;
int y = 0;

void draw(void) {
  u8g2.setFont(u8g2_font_profont12_mr);
  if (Bat_Volt < 1) {
    u8g2.setCursor(37, 44);
    u8g2.println("No Battery");
  }
  else if (Bat_Volt > Bat_High) {
    u8g2.setCursor(31, 44);
    u8g2.println("High Voltage");
  }
  else if (Bat_Volt < Bat_Low) {
    u8g2.setCursor(34, 44);
    u8g2.println("Low Voltage");
  }
  else if (Bat_Volt >= Bat_Low && Bat_Volt < Bat_High) {
    u8g2.setCursor(46, 14);
    u8g2.println("Testing");
    u8g2.drawStr(0, 28, "V: ");
    u8g2.drawStr(0, 44, "I: ");
    u8g2.drawStr(0, 60, "mAh: ");
    u8g2.setCursor(58, 28);
    u8g2.print(Bat_Volt, 2);
    u8g2.println("V");
    u8g2.setCursor(58, 44);
    u8g2.print(mA, 0);
    u8g2.println("mA");
    u8g2.setCursor(58, 60);
    u8g2.print(Capacity, 1);
  }
}

void buzz() {
  digitalWrite(Buzz_Pin, HIGH);
  delay(100);
  digitalWrite(Buzz_Pin, LOW);
  delay(10);
}

void setup() {
  Serial.begin(115200);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A4, INPUT_PULLUP);
  pinMode(A5, INPUT_PULLUP);
  pinMode(MOSFET_Pin, OUTPUT);
  pinMode(Buzz_Pin, OUTPUT);
  digitalWrite(MOSFET_Pin, LOW);
  u8g2.begin();
  Serial.print("Initializing SD card... ");
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  Serial.println("card initialized.");
}

void loop() {
  for (int i = 0; i < 100; i++) {
    sample1 = sample1 + analogRead(Vsense_P);
    delay(2);
  }
  sample1 = sample1 / 100;
  Bat_Volt = 2 * sample1 * Vcc / 1024.0;

  for (int i = 0; i < 100; i++) {
    sample2 = sample2 + analogRead(Vsense_N);
    delay(2);
  }
  sample2 = sample2 / 100;
  Res_Volt = 2 * sample2 * Vcc / 1024.0;

  if (Bat_Volt < 1) {
    digitalWrite(MOSFET_Pin, LOW);
    Serial.println("No Battery");
    delay(2000);
  }

  else if (Bat_Volt > Bat_High) {
    digitalWrite(MOSFET_Pin, LOW);
    buzz();
    Serial.print("Warning High Voltage! V = ");
    Serial.println(Bat_Volt);
    delay(1000);
  }

  else if (Bat_Volt < Bat_Low) {
    digitalWrite(MOSFET_Pin, LOW);
    Serial.print("Warning Low Voltage! V = ");
    Serial.println(Bat_Volt);
    delay(1000);
  }

  else if (Bat_Volt > Bat_Low && Bat_Volt < Bat_High) {
    digitalWrite(MOSFET_Pin, HIGH);
    millisPassed = millis() - previousMillis;
    Current = (Bat_Volt - Res_Volt) / Res_Value;
    mA = Current * 1000.0;
    Capacity = Capacity + mA * (millisPassed / 3600000.0);
    previousMillis = millis();

    //Write to SD
    File dataFile = SD.open("battery.txt", FILE_WRITE);
    if (dataFile) {
      dataFile.print(Capacity); dataFile.print(","); dataFile.println(Bat_Volt);
      dataFile.close();
      //Print in Serial Monitor
      Serial.print("DATA,TIME,"); Serial.print(Bat_Volt); Serial.print(","); Serial.println(Capacity);
    }
    else {
      Serial.println("error opening battery.txt");
    }
    y++;
    x++;
    delay(1000);
  }

  u8g2.firstPage();
  do
  {
    draw();
  } while (u8g2.nextPage());
}
