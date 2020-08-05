// Program do komunikacji radiowej 2.4 Ghz
// ______ TRANSMITTER ________
// Autor: Maciej Zawartka
// Wymagane biblioteki: RF24
// Arduino IDE 1.8.13 Stable|   Arduino Nano ATmega328P OldBootloader


// Biblioteki 
#include <Wire.h>     // obwod
#include <SPI.h>      // komunikacja SPI z nRF24L01
#include <nRF24L01.h> // Modul nRF24L01
#include <RF24.h>     // Modul nRF24L01

//DEFINIOWANIE WEJSC
#define T1 5     //Przelaczniki od lewej numerowane
#define T2 6
#define T3 8
#define T4 7

// INICJACJA RADIA
RF24 radio(10, 9);   // nRF24L01 (CE, CSN)
const byte address[6] = "00001";    //kanal do komunikacji

//DANE CZASOWE DO PETLI
#define interval_1  500 //[ms] odczytaj co okres kontrole
#define interval_2  500 //[ms] wyswietlaj
unsigned long cur_time = 0; // aktualny czas
unsigned long prev_time_1 = 0; // poprzedni czas scenariusz 1
unsigned long prev_time_2 = 0; // poprzedni czas scenariusz 2

//PACZKA DANYCH
struct Data_Package {
  //paczka danych do wyslania
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

// ======= USTAWIENIA ======
  void setup() {
  // Komunikacja szeregowa
  Serial.begin(9600);

  // Tryb przelacznikow, podciagane do +5V
  pinMode(T1, INPUT_PULLUP);
  pinMode(T2, INPUT_PULLUP);
  pinMode(T3, INPUT_PULLUP);
  pinMode(T4, INPUT_PULLUP);
  // <====================================================?????? tutaj opisac polecenia radiowe
  // Inicjacja polaczenia radiowego
  radio.begin();
  radio.openWritingPipe(address);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  
  // Offset potencjometrow
  int errorjLX = 127;  
  int errorjLY = 123;
  int errorjPX = 114;
  int errorjPY = 143;
  
  // Ustalenie danych poczatkowych
  dane.jLX = errorjLX;  //
  dane.jLY = errorjLY;
  dane.jPX = errorjPX;
  dane.jPY = errorjPY;
  dane.Tog1 = 1;
  dane.Tog2 = 1;
  dane.Tog3 = 1;
  dane.Tog4 = 1; 
}

//====== GLOWNY PROGRAM ======
void loop() {
  
  cur_time = millis();  //odczytaj aktualny czas
  if ( cur_time - prev_time_1 >= interval_1){
    // Odczyt danych do struktury co czas interval_1
    dane.jLX = map(analogRead(A1), 0, 1023, 0, 255);
    dane.jLY = map(analogRead(A0), 0, 1023, 0, 255);
    dane.jPX = map(analogRead(A3), 0, 1023, 255, 0);
    dane.jPY = map(analogRead(A2), 0, 1023, 255, 0);
    dane.Tog1 = digitalRead(T1);
    dane.Tog2 = digitalRead(T2);
    dane.Tog3 = digitalRead(T3);
    dane.Tog4 = digitalRead(T4);  
    // Wyslanie danych
    radio.write(&dane, sizeof(Data_Package));
    prev_time_1 = cur_time;
  } 
  if (cur_time - prev_time_2 >= interval_2){
    // Zarzadzanie wyswietlaczem
    prev_time_2 = cur_time;
  }
 
}
