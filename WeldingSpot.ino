#include <TM1637.h>
#include <EEPROM.h>

//pins definitions for TM1637 and can be changed to other ports    
#define TM1637_CLK 6
#define TM1637_DIO 7

#define BTN1 8
#define BTN2 9
#define SW1 11
#define DEBOUNCE 100  // таймаут антидребезга, миллисекунды

//const int buttonPin = 11;    // Кнопка (педаль) спуска
const int ledPin = 12;       // Пин с сигнальным светодиодом
const int triggerPin = 10;   // Пин управления MOSFET'ами
const int buzzerPin = 13;     // Пищалка
//const int touchPin1 = 8;      // Кнопка 1 изменения времени импульса
//const int touchPin2 = 9;      // Кнопка 2 изменения времени импульса

TM1637 tm1637(TM1637_CLK,TM1637_DIO);

int8_t TDisp[] = {0,0,4,5};

// Объявляем переменные:
int WeldingNow = LOW;
//int buttonState = LOW;;
//int touch1State = LOW;;
//int touch2State = LOW;;
//int lastButtonState = LOW;
//int lastTouch1State = LOW;
//int lastTouch2State = LOW;
boolean FootSwitch = false;
boolean ShowDelay = false;

boolean btn1State, btn2State, sw1State;
boolean btn1Flag, btn2Flag, sw1Flag;
unsigned long deb1Timer, deb2Timer, deb3Timer;

//unsigned long lastDebounceTime = 0;
//unsigned long lastTDebounceTime = 0;
//unsigned long debounceDelay = 50;    // минимальное время в мс, которое надо выждать до срабатывания. Сделано для предотвращения ложных срабатываний при дребезге контактов спусковой кнопки

byte weldingTime = 2;        // ...и на его основе выставляем задержку
// unsigned int TimeOutPulse = 20;
// unsigned int FirstPulse = 20;
byte MaxWeldingTime = 100;

void setup() {
  // put your setup code here, to run once:
  
  pinMode(BTN1,  INPUT);
  pinMode(BTN2,  INPUT);
  pinMode(SW1,  INPUT);
  pinMode(ledPin,     OUTPUT);
  pinMode(triggerPin, OUTPUT);
  pinMode(buzzerPin,  OUTPUT);
  digitalWrite(ledPin,     LOW);
  digitalWrite(triggerPin, LOW);
  digitalWrite(buzzerPin,  LOW);

  if (EEPROM.read(0) != 255) {
    weldingTime = EEPROM.read(0);
  }

  if (EEPROM.read(1) != 255) {
    FootSwitch = EEPROM.read(1);
  }


//  Serial.begin(9600);
  tm1637.init();
  tm1637.set(BRIGHT_TYPICAL);
  tm1637.display(TDisp);
  delay(1000);

  ShowDelay = true;
}

void loop() {
  // put your main code here, to run repeatedly:

  // читаем значения сенсоров

  btn1State = digitalRead(BTN1);  // читаем состояние кнопки с инверсией. 1 - нажата, 0 - нет  
  btn2State = digitalRead(BTN2);  // читаем состояние кнопки с инверсией. 1 - нажата, 0 - нет
  sw1State = digitalRead(SW1);  // читаем состояние педали с инверсией. 1 - нажата, 0 - нет
    
  if (btn1State && !btn1Flag && (millis() - deb1Timer > DEBOUNCE)) {
    btn1Flag = true;              // запомнили что нажата
    deb1Timer = millis();    // запомнили время нажатия
    weldingTime = weldingTime + 1;
  }
  if (!btn1State && btn1Flag) {    // если отпущена и была нажата (btnFlag 1)
    btn1Flag = false;             // запомнили что отпущена
    deb1Timer = millis();    // запомнили время отпускания
    ShowDelay = true;
  }
  if (weldingTime > MaxWeldingTime) weldingTime = 2;
  


  if (btn2State && !btn2Flag && (millis() - deb2Timer > DEBOUNCE)) {
    btn2Flag = true;              // запомнили что нажата
    deb2Timer = millis();    // запомнили время нажатия
    weldingTime = weldingTime - 1;
  }
  if (!btn2State && btn2Flag) {    // если отпущена и была нажата (btnFlag 1)
    btn2Flag = false;             // запомнили что отпущена
    deb2Timer = millis();    // запомнили время отпускания
    ShowDelay = true;    
  }
  if (weldingTime <= 1) weldingTime = 100;


  if (sw1State && !sw1Flag && (millis() - deb3Timer > DEBOUNCE)) {
    sw1Flag = true;              // запомнили что нажата
    deb3Timer = millis();    // запомнили время нажатия
    if (btn2Flag == true) {
      FootSwitch = !FootSwitch;
      TDisp[0] = 15;
      TDisp[1] = 0;
      TDisp[2] = 0;
      TDisp[3] = FootSwitch;
      tm1637.display(TDisp);
      delay(1000);
      weldingTime = weldingTime + 1;
//      ShowDelay = true;      
    } else WeldingNow = !WeldingNow; // запускаем сварку
  }
  if (!sw1State && sw1Flag) {    // если отпущена и была нажата (sw1Flag 1)
    sw1Flag = false;             // запомнили что отпущена
    deb3Timer = millis();    // запомнили время отпускания
  }


//  int touch1 = digitalRead(touchPin1);
//  int touch2 = digitalRead(touchPin2);
//  int reading = digitalRead(buttonPin);


//  sensorValue = analogRead(analogPin); // считываем значение, выставленное на потенциометре
//  weldingTime = map(sensorValue, 0, 1023, 15, 255); // приводим его к миллисекундам в диапазоне от 15 до 255

  if (ShowDelay == true) {
    TDisp[0] = 13;
    TDisp[1] = weldingTime/100;
    TDisp[2] = weldingTime%100/10;
    TDisp[3] = weldingTime%10;
    tm1637.display(TDisp);
  }
  

    // Если команда получена, то начинаем:
  if (WeldingNow == HIGH) {

  //  Serial.println("== Welding starts now! ==");
  //    delay(1000);

//    TDisp[0] = 15;
//    TDisp[1] = 1;
//    TDisp[2] = 10;
//    TDisp[3] = 0;
//    tm1637.display(TDisp);

  if (FootSwitch == false) {
    // Выдаём три коротких и один длинный писк в динамик:
    int cnt = 1;
    while (cnt <= 3) {
      playTone(1915, 150); // другие ноты на выбор: 1915, 1700, 1519, 1432, 1275, 1136, 1014, 956
      delay(500);
      cnt++;
    }
    playTone(956,  300);
    delay(1);
  }

    // Первый импульс
//    digitalWrite(ledPin,     HIGH);
//    digitalWrite(triggerPin, HIGH);
//    if (weldingTime > 160) delay (weldingTime/8);
//      else delay (FirstPulse);
//    digitalWrite(triggerPin, LOW);
//    digitalWrite(ledPin,     LOW);

//    if (weldingTime > 160) delay (weldingTime/8);
//      else delay (FirstPulse);
    
    // И сразу после последнего писка приоткрываем MOSFET'ы на нужное количество миллисекунд:
    digitalWrite(ledPin,     HIGH);
    digitalWrite(triggerPin, HIGH);
    delay(weldingTime);
    digitalWrite(triggerPin, LOW);
    digitalWrite(ledPin,     LOW);

    EEPROM.update(0,weldingTime);
    EEPROM.update(1,FootSwitch);
    
//    TDisp[0] = 14;
//    TDisp[1] = 1;
//    TDisp[2] = 1;
//    TDisp[3] = 13;
//    tm1637.display(TDisp);
    
    // Serial.println("== Welding ended! ==");
    delay(1000);

    // И всё по-новой:
    WeldingNow = LOW;

  } else {
    digitalWrite(ledPin,     LOW);
    digitalWrite(triggerPin, LOW);
    digitalWrite(buzzerPin,  LOW);
  }

//  lastButtonState = reading;
//  lastTouch1State = touch1;
//  lastTouch2State = touch2;

  ShowDelay = false;

}

// В эту функцию вынесен код, обслуживающий пищалку:
void playTone(int tone, int duration) {
//  digitalWrite(ledPin, HIGH);
  for (long i = 0; i < duration * 1000L; i += tone * 2) {
    digitalWrite(buzzerPin, HIGH);
    delayMicroseconds(tone);
    digitalWrite(buzzerPin, LOW);
    delayMicroseconds(tone);
  }
//  digitalWrite(ledPin, LOW);
}
