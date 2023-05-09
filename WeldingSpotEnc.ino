#include <TM1637.h>
#include <EEPROM.h>
#include <EncButton.h>
#include <EEManager.h>

//pins definitions for TM1637 and can be changed to other ports    
#define TM1637_CLK 6
#define TM1637_DIO 7

#define ENC_1 2
#define ENC_2 3
#define ENC_BTN 4
#define PEDAL_PIN 11 // Кнопка (педаль) спуска

// структура для хранения данных
struct Data {
  byte weldingTime;
  unsigned long f_count;
};
Data ee_data;  // переменная, с которой мы работаем в программе

EEManager memory(ee_data); // передаём нашу переменную (фактически её адрес)

const byte ledPin = 13;       // Пин с сигнальным светодиодом
const byte triggerPin = 10;   // Пин управления MOSFET'ами
const byte buzzerPin = 12;    // Пищалка

const byte MaxWeldingTime = 99; // Максимальное время импульса


TM1637 tm1637(TM1637_CLK,TM1637_DIO);

EncButton<EB_TICK, ENC_1, ENC_2, ENC_BTN> my_enc;
//EncButton2<EB_ENCBTN> my_enc(INPUT, ENC_1, ENC_2, ENC_BTN);  // энкодер с кнопкой

EncButton<EB_TICK, PEDAL_PIN> my_pedal;
//EncButton2<EB_BTN> my_pedal(INPUT_PULLUP, PEDAL_PIN);

int8_t TDisp[] = {11,0x7f,0x7f,3};

// Объявляем переменные:
boolean ShowDelay = false;

unsigned long Ver_Timer = 0;

byte weldingTime = 1;

void setup() {
  // put your setup code here, to run once:

  memory.begin(0, 'V');

  //my_enc.setEncType(EB_HALFSTEP); // тип энкодера: EB_FULLSTEP (0) по умолч., EB_HALFSTEP (1) если энкодер делает один поворот за два щелчка
  //my_enc.setButtonLevel(HIGH);     // уровень кнопки: LOW - кнопка подключает GND (по умолч.), HIGH - кнопка подключает VCC
  //my_pedal.setButtonLevel(HIGH);     // уровень кнопки: LOW - кнопка подключает GND (по умолч.), HIGH - кнопка подключает VCC
  
  pinMode(ledPin,     OUTPUT);
  pinMode(triggerPin, OUTPUT);
  pinMode(buzzerPin,  OUTPUT);
  digitalWrite(ledPin,     LOW);
  digitalWrite(triggerPin, LOW);
  digitalWrite(buzzerPin,  LOW);

//  if (EEPROM.read(0) != 255) {
//    weldingTime = EEPROM.read(0);
//  }

  weldingTime = ee_data.weldingTime;

//  if (EEPROM.read(1) != 255) {
//    FootSwitch = EEPROM.read(1);
//  }


//  Serial.begin(9600);
  tm1637.init();
  tm1637.set(BRIGHT_TYPICAL);
  tm1637.display(TDisp);
//  delay(1000);

  start_beep();

//  ShowDelay = true;
  Ver_Timer = (millis() + 1000);
}

void loop() {
  // put your main code here, to run repeatedly:

   memory.tick();

   my_enc.tick();                     // опрос энкодера

   my_pedal.tick();                     // опрос педали


  if((Ver_Timer > 0) && (Ver_Timer < millis())) {
    ShowDelay = true;
    Ver_Timer = 0;
  }

  if (my_enc.left()) { // поворот налево

    if(weldingTime < MaxWeldingTime) {
      weldingTime = weldingTime + 1;
      ShowDelay = true; }
    else
      err_beep();
  }

  if (my_enc.right()) { // поворот направо

    if(weldingTime > 1) {
      weldingTime = weldingTime - 1;
      ShowDelay = true; }
    else
      err_beep();
  }

  if (my_enc.press()) {
      TDisp[0] = 15;
      TDisp[1] = ee_data.f_count/100;
      TDisp[2] = ee_data.f_count%100/10;
      TDisp[3] = ee_data.f_count%10;
      tm1637.display(TDisp);
      ShowDelay = false;
      Ver_Timer = (millis() + 2000);
      start_beep();
  }


  if(my_pedal.press()) {

      TDisp[0] = 15;
      TDisp[1] = ee_data.f_count/100;
      TDisp[2] = ee_data.f_count%100/10;
      TDisp[3] = ee_data.f_count%10;
      tm1637.display(TDisp);
      ShowDelay = false;
      Ver_Timer = (millis() + 2000);

  // Выдаём три коротких и один длинный писк в динамик:
  //   byte cnt = 1;
  //    while (cnt <= 3) {
  //      playTone(1915, 150); // другие ноты на выбор: 1915, 1700, 1519, 1432, 1275, 1136, 1014, 956
  //      delay(500);
  //      cnt++;
  //    }
  //    playTone(956,  300);
  //    delay(1);


      playTone(1915, 150); // другие ноты на выбор: 1915, 1700, 1519, 1432, 1275, 1136, 1014, 956

    // И сразу после последнего писка приоткрываем MOSFET'ы на нужное количество миллисекунд:

    // Первый импульс
//    digitalWrite(ledPin,     HIGH);
//    digitalWrite(triggerPin, HIGH);
//    if (weldingTime > 160) delay (weldingTime/8);
//      else delay (FirstPulse);
//    digitalWrite(triggerPin, LOW);
//    digitalWrite(ledPin,     LOW);

//    if (weldingTime > 160) delay (weldingTime/8);
//      else delay (FirstPulse);

    // Основной импульс
    digitalWrite(ledPin,     HIGH);
    digitalWrite(triggerPin, HIGH);
    delay(weldingTime);
    digitalWrite(triggerPin, LOW);
    digitalWrite(ledPin,     LOW);
    digitalWrite(buzzerPin,  LOW);

    ee_data.weldingTime = weldingTime;
    ee_data.f_count++;
    memory.update();
//    EEPROM.update(0,weldingTime);
//    EEPROM.update(1,FootSwitch);
    
  }


  if (ShowDelay == true) {
    TDisp[0] = 13;
//    TDisp[1] = weldingTime/100;
    TDisp[1] = 0x7f;
    TDisp[2] = weldingTime%100/10;
    TDisp[3] = weldingTime%10;
    tm1637.display(TDisp);
    ShowDelay = false;
  }
  

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

void err_beep() {

  playTone(1014, 100); // другие ноты на выбор: 1915, 1700, 1519, 1432, 1275, 1136, 1014, 956

}


void start_beep() {

  playTone(1519, 200); // другие ноты на выбор: 1915, 1700, 1519, 1432, 1275, 1136, 1014, 956

}
