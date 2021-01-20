/*
Reaction Time Meter
by EFindus AKA. adammickiewicz635

This code allows you check your reaction time.

Created on 10 January 2021
by EFindus

*/

//BUTTONS
#define MAIN_BUTTON 4

//LED
#define MAIN_LED 5

void setup() {
  Serial.begin(9600);                                           // Inicjalizujemy komunikację portem szeregowym w celu wyświetlania wyniku
  
  pinMode(MAIN_BUTTON, INPUT_PULLUP);                           // Inicjalizujemy porty w płytce

  pinMode(MAIN_LED, OUTPUT);

  digitalWrite(MAIN_LED, HIGH);                                 // Wyłączamy diodę LED
}

void loop() {
  float Result = 0;                                             // Resetujemy zmienne
  digitalWrite(MAIN_LED, HIGH);                                 // Wyłączamy diodę LED
  randomSeed(analogRead(0));                                    // Inicjalzujemy generator liczb pseudolosowych (stan na niezinicjalizowanym porcie analogowym jest losowy)

  for (int Attempt = 1; Attempt <= 5; Attempt++) {              // Pięć testów
    int RandomDelay = random(2000, 5000);                       // Generujemy czas po jakim zapali się dioda
    for (int Delay = 0; Delay < RandomDelay; Delay++) {         // Odczekujemy wylosowany czas (ta pętla zostanie wykonana tyle razy, i za każdym razem program poczeka 1 milisekundę)
      if (digitalRead(MAIN_BUTTON) == LOW){                     // Jeżeli użytkownik nacisnął przycisk przed zapaleniem się diody resetujemy odliczanie
        delay(20);
        while(digitalRead(MAIN_BUTTON) == LOW);
        delay(20);
        Delay = 0;
      }
      
      delay(1);
    }

    digitalWrite(MAIN_LED, LOW);                                // Gasimy diodę LED

    while(digitalRead(MAIN_BUTTON) == HIGH) {                   // Czekamy aż użytkownik naciśnie przycisk
      Result += 0.001;                                          // Liczymy czas jaki czekamy na naciśnięcie do wyniku
      delay(1);
    }
    
    delay(20);
    while(digitalRead(MAIN_BUTTON) == LOW);
    delay(20);

    digitalWrite(MAIN_LED, HIGH);
  }

  Serial.println(Result / 5);                                   // Uśreniamy i wysyłamy wynik na port szeregowy USB
  
  while(digitalRead(MAIN_BUTTON) == HIGH);                      // Czekamy na ponowne naciśnięcie przycisku aby powtórzyć test
  delay(20);
  while(digitalRead(MAIN_BUTTON) == LOW);
  delay(20);
}
