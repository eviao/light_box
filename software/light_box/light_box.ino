#include <MsTimer2.h>
#include <EEPROM.h>
#include <SevSeg.h>
#include <Bounce2.h>

#define INCREASE_PIN    A0
#define DECREASE_PIN    A1
#define START_PIN       A2
#define RELAY_PIN       A3

#define TIME_ADDR   0x0000

#define MAX_TIME    9999
#define MIN_TIME    0

SevSeg sevseg;

Bounce increaseDebouncer = Bounce();
Bounce decreaseDebouncer = Bounce();
Bounce startDebouncer = Bounce();

bool starting = false;
int timer = 0;

void initSevSeg() {
  
  byte digitPins[] = { 13, 12, 11, 10 };
  byte segmentPins[] = { 9, 8, 7, 6, 5, 4, 3, 2 };

  sevseg.begin(COMMON_CATHODE, 4, digitPins, segmentPins, false, false, true, true);
  sevseg.setBrightness(90);
}

void initButtons() {
  increaseDebouncer.attach(INCREASE_PIN, INPUT_PULLUP);
  decreaseDebouncer.attach(DECREASE_PIN, INPUT_PULLUP);
  startDebouncer.attach(START_PIN, INPUT_PULLUP);
  
  increaseDebouncer.interval(50);
  decreaseDebouncer.interval(50);
  startDebouncer.interval(50);
}

void handleTiming() {
  timer -= 1;
  
  if (timer <= 0) {
    starting = false;
    digitalWrite(RELAY_PIN, HIGH);
    MsTimer2::stop();
    loadTimer();
  }
}

void setup() {
  Serial.begin(9600);

  loadTimer();

  initSevSeg();
  initButtons();

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);

  MsTimer2::set(1000,  handleTiming);
}

void loadTimer() {
  EEPROM.get(TIME_ADDR, timer);
}

void saveTimer() {
  if (timer < MIN_TIME || timer > MAX_TIME) {
    timer = 0;
  }
  EEPROM.write(TIME_ADDR, timer);
}

void loop() {

  if (!starting) {
    if (timer > 0) {
      startDebouncer.update();
      if (startDebouncer.fell()) {
        MsTimer2::start();
        digitalWrite(RELAY_PIN, LOW);
        starting = true;
      }
    }
    
    increaseDebouncer.update();
    if (increaseDebouncer.fell()) {
      timer += 30;
      saveTimer();
    }

    decreaseDebouncer.update();
    if (decreaseDebouncer.fell()) {
      timer -= 30;
      saveTimer();
    }
  }

  sevseg.setNumber(timer, -1);
  sevseg.refreshDisplay();
}
