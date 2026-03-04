#include "ssd1306-smalltext-wire.h"
#include <Wire.h>
#include <Servo.h>
#include <SensirionI2cScd4x.h>

// izmantotie pini
const uint8_t Button1 = 2;
const uint8_t Button2 = 3;
const uint8_t Button3 = 4;

const uint8_t ServoMotor = 9;

const uint8_t InfoLed = 13; // built-in

// 0: nav datu
// 1: lielisks līmenis, < 800
// 2: neitrāls līmenis, < 1300
// 3: slikts līmenis, >= 1300
uint8_t overall_co2_level = 0;

SensirionI2cScd4x sensor;
Servo servo;

void blink_powerup() {
  digitalWrite(InfoLed, 1);
  delay(50);
  digitalWrite(InfoLed, 0);
  delay(50);
  digitalWrite(InfoLed, 1);
  delay(50);
  digitalWrite(InfoLed, 0);
  delay(50);
  digitalWrite(InfoLed, 1);
  delay(100);
  digitalWrite(InfoLed, 0);
  delay(50);
}

bool initialize_sensor()
{
  int16_t error;


  sensor.begin(Wire, SCD41_I2C_ADDR_62);
  delay(30);

    // Ensure sensor is in clean state
  error = sensor.wakeUp();
  if (error != 0) {
    text1("err: wakeup");
    return false;
  }

  error = sensor.stopPeriodicMeasurement();
  if (error != 0) {
    text1("err: stop");
    return false;
  }

  error = sensor.reinit();
  if (error != 0) {
    text1("err: reinit");
    return false;
  }

  error = sensor.startPeriodicMeasurement();
  if (error != 0) {
    text1("err: startPeriodic");
    return false;
  }

  text1("Sensor ready");

  return true;
}

void setup () {


  Wire.begin();
  Wire.setClock(100000);

  overall_co2_level = 0;

  pinMode(Button1, INPUT_PULLUP);
  pinMode(Button2, INPUT_PULLUP);
  pinMode(Button3, INPUT_PULLUP);

  pinMode(InfoLed, OUTPUT);

  blink_powerup();

  ssd1306_init();

  text0("Cesis+Daugavpils");
  text1("");
  text2("");
  text3("");

  initialize_sensor();

}

uint8_t co2_override = 0;
unsigned long co2_override_until = 0;


bool update_measure ()
{
  if (co2_override) {
    if (millis() < co2_override_until) {
      overall_co2_level = co2_override;
      return true;
    }
  }

  bool data_ready = false;

  float temperature = 0.0;
  float rel_humidity = 0.0;
  uint16_t co2 = 0;

  if (0 != sensor.getDataReadyStatus(data_ready)) {
    text1("err: getdataready");
    return false;
  }
  if ( ! data_ready) return false;

  digitalWrite(InfoLed, 1);

  if (0 != sensor.readMeasurement(co2, temperature, rel_humidity)) {
    text1("err: read");
    return false;
  }

  co2_override = 0;
  co2_override_until = 0;

  char buf[16];
  sprintf(buf, "CO2 %d %d.%dC",
    co2,
    (int)temperature,
    (int)((temperature - (int)temperature) * 10));
  text2(buf);


  if (co2 < 700) {
    overall_co2_level = 1;
  } else if (co2 < 1300) {
    overall_co2_level = 2;
  } else {
    overall_co2_level = 3;
  }
  return true;
}



uint8_t prev_level = 0;

void loop() {
  if ( ! update_measure()) {
    digitalWrite(InfoLed, 0);
  }

  if (overall_co2_level != prev_level) {
    prev_level = overall_co2_level;

    switch(overall_co2_level) {
    case 0:
      text3("no data");
      break;
    case 1:
      text3("EXCELLENT");
      servo.attach(ServoMotor);
      servo.write(180);
      delay(400);
      servo.detach();
      break;
    case 2:
      text3("GOOD");
      servo.attach(ServoMotor);
      servo.write(90);
      delay(400);
      servo.detach();
      break;
    case 3:
      text3("BAD");
      servo.attach(ServoMotor);
      servo.write(0);
      delay(400);
      servo.detach();
      break;
    }
  }

  if ( ! digitalRead(Button1)) {
    co2_override = 1;
    co2_override_until = millis() + 5000; // 5s
  }
  if ( ! digitalRead(Button2)) {
    co2_override = 2;
    co2_override_until = millis() + 5000; // 5s
  }
  if ( ! digitalRead(Button3)) {
    co2_override = 3;
    co2_override_until = millis() + 5000; // 5s
  }
  delay(100);
}

