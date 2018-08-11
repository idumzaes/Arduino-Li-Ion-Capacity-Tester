#include <VirtualWire.h>
#include <U8glib.h>
U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NONE);  // I2C interface for OLED

#define MOSFET_Pin 3    //Mosfet Gate pin
#define Buzz_Pin 9      //Buzzer pin
#define Vsense_P A0     //Positive Voltage Sense
#define Vsense_N A1     //Negative Voltage Sense
#define TxPin 12        //433Mhz transmitter pin
#define TxLED 13        //433Mhz Activity LED
char Msg[30];           //433Mhz string to be sent

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
  u8g.setFont(u8g_font_fub14r);
  if (Bat_Volt < 1) {
    u8g.setPrintPos(10, 40);
    u8g.println("No Battery");
  }
  else if (Bat_Volt > Bat_High) {
    u8g.setPrintPos(3, 40);
    u8g.println("High Voltage");
  }
  else if (Bat_Volt < Bat_Low) {
    u8g.setPrintPos(3, 40);
    u8g.println("Low Voltage");
  }
  else if (Bat_Volt >= Bat_Low && Bat_Volt < Bat_High) {
    u8g.drawStr(0, 20, "V: ");
    u8g.drawStr(0, 40, "I: ");
    u8g.drawStr(0, 60, "mAh: ");
    u8g.setPrintPos(58, 20);
    u8g.print(Bat_Volt, 2);
    u8g.println("V");
    u8g.setPrintPos(58, 40);
    u8g.print(mA, 0);
    u8g.println("mA");
    u8g.setPrintPos(58, 60);
    u8g.print(Capacity, 1);
  }
}

void buzz() {
  digitalWrite(Buzz_Pin, HIGH);
  delay(100);
  digitalWrite(Buzz_Pin, LOW);
  delay(10);
}

void setup() {
  Serial.begin(9600);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A4, INPUT_PULLUP);
  pinMode(A5, INPUT_PULLUP);
  pinMode(MOSFET_Pin, OUTPUT);
  pinMode(Buzz_Pin, OUTPUT);
  digitalWrite(MOSFET_Pin, LOW);
  // VirtualWire setup
  pinMode(TxLED, OUTPUT);
  vw_setup(2000);         //RF Bits per sec
  vw_set_tx_pin(TxPin);   //Set the Tx pin

  sprintf(Msg, "Power On");
  digitalWrite(TxLED, HIGH);  //LED on Tx initiated
  delay(2000);
  vw_send((uint8_t *)Msg, strlen(Msg));
  vw_wait_tx();               //Wait for Tx completion
  digitalWrite(TxLED, LOW);   //LED off Tx completed
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

    //433Mhz Tx
    sprintf(Msg, "%d,%d", Bat_Volt * 100, Capacity * 100);
    digitalWrite(TxLED, HIGH);  //LED on Tx initiated
    delay(1000);
    vw_send((uint8_t *)Msg, strlen(Msg));
    vw_wait_tx();               //Wait for Tx completion
    digitalWrite(TxLED, LOW);   //LED off Tx completed
    //Print in Serial Monitor
    Serial.print("V,"); Serial.print(Bat_Volt); Serial.print(",mAh,"); Serial.println(Capacity * 100);

    y++;
    x++;
    delay(100);
  }

  u8g.firstPage();
  do
  {
    draw();
  } while (u8g.nextPage());
}
