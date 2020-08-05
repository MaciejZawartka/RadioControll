// Program do komunikacji radiowej 2.4 Ghz
// ______ RECIVER ________
// Autor: Maciej Zawartka
// Wymagane biblioteki: RF24
// Arduino IDE 1.8.13 Stable   |   Arduino Nano ATmega328P OldBootloader
/*
TABELA PODLACZEN MOSTEK H:
| ARD NANO   |  L298N  |
|____________|_________|
|     EA     |   D8    |
|     EN1    |   D7    |
|     EN2    |   D6    |
|     EN3    |   D5    |
|     EN4    |   D4    |
|     EB     |   D3    |
|    +5V     |  5Vin   |
|    GND     |  GND    |

TABELA PODLACZEN nRF24L01:
| ARD NANO   | nRF24L01|
|____________|_________|
|     VIN    |   3.3V  | Filtrowane 10nF 
|     GND    |   GND   |
|     CE     |   D10   |
|     CSN    |   D9    |
|     MOSI   |   D11   |
|     SCK    |   D13   |
|     MISO   |   D12   |
|     IQR    |   np    |
*/

// Biblioteki 
#include <Wire.h>     // obwod
#include <SPI.h>      // komunikacja SPI z nRF24L01
#include <nRF24L01.h> // Modul nRF24L01
#include <RF24.h>     // Modul nRF24L01


//== DANE ==
#define enA 6
#define enB 3
#define En1 7
#define En2 8
#define En3 5
#define En4 4
  
// Offset potencjometrow
byte errorjLX = 127;  
byte errorjLY = 123;
byte errorjPX = 114;
byte errorjPY = 143;

int motorPWM_L = 0;
int motorPWM_R = 0;

byte motorbound = 80;

byte turn_signal;

//Dane do petli czasowych
#define interval_message  200 //[ms] czas oczekiwania

unsigned long cur_time = 0; // aktualny czas
unsigned long last_recive = 0; // poprzedni czas scenariusz 1


// INICJACJA RADIA
RF24 radio(10, 9);   // nRF24L01 (CE, CSN)
const byte address[6] = "00001";    //kanal do komunikacji

//PACZKA DANYCH
struct Data_Package {
  //paczka danych 
  byte jLX;
  byte jLY;
  byte jPX;
  byte jPY;
  byte Tog1;
  byte Tog2;
  byte Tog3;
  byte Tog4;
};
Data_Package dane;  //inicjacja paczki danych

void setup() {
  // <--------------------------------------------------------------------------- opisac tutaj
  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.startListening(); //  Tryb odbiornika
  dane_erase();
  // wyjscia sterowania L298N
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(En1, OUTPUT);
  pinMode(En2, OUTPUT);
  pinMode(En3, OUTPUT);
  pinMode(En4, OUTPUT);
}
// ============== MAIN ==========
void loop() {
  lostsignal();
  checksignal();
  drive(); 
  print_all();
  
}
  
// ==================================== FUNKCJE WLASNE =====================
void checksignal(){
  // Sprawdzenie dostepnosci sygnalu
  if (radio.available()) {
    radio.read(&dane, sizeof(Data_Package)); // odczyt i zapis struktury danych
    last_recive = millis(); // ostatnia wiadomosc
  }
}
//---------------------------------------------------
void lostsignal(){
  // zabezpieczenie przed utrata sygnalu
  cur_time = millis();
  if (cur_time - last_recive >= interval_message ){
    dane_erase();
  }
}
//---------------------------------------------------
void dane_erase(){
  // usuwanie danych
  dane.jLX = errorjLX;  
  dane.jLY = errorjLY;
  dane.jPX = errorjPX;
  dane.jPY = errorjPY;
  dane.Tog1 = 1;
  dane.Tog2 = 1;
  dane.Tog3 = 1;
  dane.Tog4 = 1; 
}

//----------------------------------------------------
void print_all(){
  // Wyswietlanie danych
  Serial.print("POT1 X:   ");
  Serial.print(dane.jLX);
  Serial.print("   POT1 Y:   ");
  Serial.print(dane.jLY);
  Serial.print("   POT2 X:   ");
  Serial.print(dane.jPX);
  Serial.print("   POT2 Y:   ");
  Serial.println(dane.jPY);
//  Serial.print("TOG1:   ");
//  Serial.print(dane.Tog1);
//  Serial.print("TOG2:   ");
//  Serial.print(dane.Tog2);
//  Serial.print("TOG3:   ");
//  Serial.print(dane.Tog3);
//  Serial.print("TOG4:   ");
//  Serial.println(dane.Tog4);
  delay(200);
}
//----------------------------------------------------
void drive(){
  // Przygotowanie sygnalu do jazdy na wprost
  if (dane.jLY > 1.2 * errorjLY){
    // Jazda do przodu
    // Lewy silnik do przodu
    digitalWrite(En1, LOW);
    digitalWrite(En2, HIGH);
    // Prawy silnik do przodu
    digitalWrite(En3, LOW);
    digitalWrite(En4, HIGH);
    //
    motorPWM_L = map(dane.jLY, 1.2 * errorjLY, 255, motorbound, 255);
    motorPWM_R = map(dane.jLY, 1.2 * errorjLY, 255, motorbound, 255);
   
  }
  else if (dane.jLY < 0.8 * errorjLY){
    // Jazda do tylu
    // Lewy silnik do tylu
    digitalWrite(En1, HIGH);
    digitalWrite(En2, LOW);
    // Prawy silnik do tylu
    digitalWrite(En3, HIGH);
    digitalWrite(En4, LOW);
    motorPWM_L = map(dane.jLY, 0 ,0.8 * errorjLY, 255, motorbound);
    motorPWM_R = map(dane.jLY, 0 ,0.8 * errorjLY, 255, motorbound);
  }
  else{
    //Warunek postoju/hamowania
    motorPWM_L = 0;
    motorPWM_R = 0;
  }
  // Przygotowanie sygnalu do skretu
  if (dane.jLX > 1.2 * errorjLX){
    //skret prawo
    turn_signal = map(dane.jLX, 1.2 * errorjLX, 255, 0, 255);
    motorPWM_L = motorPWM_L + turn_signal;
    motorPWM_R = motorPWM_R - turn_signal;
    if (motorPWM_L > 255){   motorPWM_L =255; } 
    else if (motorPWM_R < 255){   motorPWM_R = 0; }
  }
  else if ((dane.jLX < 0.8 * errorjLX)){
    //skret lewo
    turn_signal = map(dane.jLX, 0, 0.8 * errorjLX, 255, 0);
    motorPWM_L = motorPWM_L - turn_signal;
    motorPWM_R = motorPWM_R + turn_signal;
    if (motorPWM_R > 255){   motorPWM_R =255; } 
    else if (motorPWM_L < 255){   motorPWM_L = 0; }
  }


  
  // wygenerowanie sygnalu PWM do sterowania silnikami
  analogWrite(enA, motorPWM_L); 
  analogWrite(enB, motorPWM_R);
}
