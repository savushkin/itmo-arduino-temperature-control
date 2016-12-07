#include <OneWire.h>
#include <LiquidCrystal.h>

#define BUTTON_SELECT 1
#define BUTTON_RIGHT 2
#define BUTTON_UP 3
#define BUTTON_DOWN 4
#define BUTTON_LEFT 5

#define MAX_TEMP 35.00
#define MIN_TEMP 18.00

#define STEP 0.5

#define SENSOR_COUNT 1

OneWire  ds[] = {OneWire(11), OneWire(3)};
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
float temp = 20.00, new_temp;

int get_button(int val) {
  if (val < 50) return BUTTON_RIGHT; //select ~ 737, left ~ 502, up ~ 143, down ~ 327, right ~ 0
  else if (val < 200) return BUTTON_UP;
  else if (val < 400) return BUTTON_DOWN;
  else if (val < 600) return BUTTON_LEFT;
  else if (val < 800) return BUTTON_SELECT;
  return 0;
}

float get_avg_temp() {
  float sum = 0;
  for (int j; j < SENSOR_COUNT; j++) {
    byte i;
    byte present = 0;
    byte type_s;
    byte data[12];
    byte addr[8];
    float celsius;

    if ( !ds[j].search(addr)) {
      ds[j].reset_search();
      delay(250);
      return;
    }

    ds[j].reset();
    ds[j].select(addr);
    ds[j].write(0x44, 1);

    delay(1000);
    present = ds[j].reset();
    ds[j].select(addr);
    ds[j].write(0xBE);

    for ( i = 0; i < 9; i++) {
      data[i] = ds[j].read();
    }

    int16_t raw = (data[1] << 8) | data[0];
    if (type_s) {
      raw = raw << 3;
      if (data[7] == 0x10) {
        raw = (raw & 0xFFF0) + 12 - data[6];
      }
    } else {
      byte cfg = (data[4] & 0x60);
      if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
      else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
      else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
      // default is 12 bit resolution, 750 ms conversion time
    }
    celsius = (float)raw / 16.0;
    sum += celsius;
  }
  return sum;
}

void setup(void) {
  lcd.begin(16, 2);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop(void) {
  int button = 0;
  int cur = 0;
  int status = 0;
  button = get_button(analogRead(0));

  if (button == BUTTON_SELECT) {
    new_temp = temp;
    lcd.clear();
    delay(1500);
    for (;;) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Select target T:");
      if (cur) {
        lcd.setCursor(0, 1);
        lcd.print(new_temp);
        cur = !cur;
      } else {
        lcd.setCursor(0, 1);
        lcd.print("     ");
        cur = !cur;
      }
      button = get_button(analogRead(0));
      if (button == BUTTON_UP && new_temp <= MAX_TEMP) new_temp += STEP;
      else if (button == BUTTON_DOWN && new_temp >= MIN_TEMP) new_temp -= STEP;
      else if (button == BUTTON_SELECT) {
        temp = new_temp;
        button = 0;
        lcd.clear();
        break;
      }
      delay(500);
    }
  }

  float current_temp = get_avg_temp();
  if (current_temp < temp) {
    status = 1;
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    status = 0;
    digitalWrite(LED_BUILTIN, LOW);
  }

  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.setCursor(3, 0);
  lcd.print(current_temp);
  lcd.setCursor(9, 0);
  lcd.print("C");
  lcd.setCursor(11, 0);
  lcd.print("S:");
  lcd.setCursor(14, 0);
  if (status == 1) lcd.print("UP");
  else if (status == 0) lcd.print("NO");
  lcd.setCursor(0, 1);
  lcd.print("Target T:");
  lcd.setCursor(10, 1);
  lcd.print(temp);
}
