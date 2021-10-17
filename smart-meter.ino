#include <DFRobot_sim808.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 13, en = 12, d4 = 6, d5 = 5, d6 = 4, d7 = 3;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

#define balance_lower_limit 2.00
#define relay_pin  7

//Mobile phone number, need to change
#define PHONE_NUMBER "9779863625138"
//
//The content of messages sent
#define MESSAGE  "The energy meter's balance is low. Please recharge soon or the power will be CUT. Thank you!"

#define PIN_TX    10
#define PIN_RX    11
SoftwareSerial mySerial(PIN_TX, PIN_RX);
DFRobot_SIM808 sim808(&mySerial);//Connect RX,TX,PWR

float consumed = 0.0;
int imp_per_s = 1600;

unsigned long lastMillis;

bool message_sent = false;

float currentBal = 0;

void setup()
{
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Smart Energy Meter");

  mySerial.begin(9600);
  Serial.begin(9600);

  //******** Initialize sim808 module *************
  while (!sim808.init())
  {
    Serial.print("Sim808 init error\r\n");
    delay(1000);
  }
  delay(3000);

  pinMode(2, INPUT_PULLUP);
  attachInterrupt(0, doCount, FALLING);

  pinMode(relay_pin, OUTPUT);

  currentBal = readBalance();
  Serial.print("The current balance is: ");
  Serial.println(currentBal);

  lcd.print("Balance: ");
  //  lcd.setCursor(0, 11);
  lcd.print(currentBal);
}

void loop()
{
  if (millis() - lastMillis >= 2 * 60 * 1000UL)
  {
    lastMillis = millis();  //get ready for the next iteration
    readBalance();
  }

  if (currentBal <= balance_lower_limit) {
    lcd.clear();
    lcd.print("Balance low");
    relayOff();
    if (!message_sent) {
      sim808.sendSMS(PHONE_NUMBER, MESSAGE);
      message_sent = true;
    }
  }
  else {
    message_sent = false;

    lcd.clear();
    lcd.print("Balance: ");
    lcd.print(currentBal);
    lcd.setCursor(0, 1);
    lcd.print("Units: ");
    lcd.print(consumed);
  }
}

float readBalance() {
  char command[] = "*400#";
  char response[200], resultcode[5];
  char* balance;
  float fBal;

  Serial.println("Checking balance");
  while (!sim808.sendUSSDSynchronous(command, resultcode, response)) {
    Serial.print('Failed to query balance');
  }

  Serial.print("The balance is:");
  for (int i = 20; i < 24; i++) {
    balance[i - 20] = response[i];
  }

  fBal = (float)atof(balance);

  return fBal;
}

void doCount() {
  Serial.println(consumed);
  consumed += 1 / imp_per_s;
}

void relayOff() {
  digitalWrite(relay_pin, LOW);
}

void relayOn() {
  digitalWrite(relay_pin, HIGH);
}
