#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

#define RST_PIN 9
#define SS_PIN 10

MFRC522 mfrc522(SS_PIN, RST_PIN);

// Substitua pelo seu cartão principal.
String MasterTag = "0A C6 3A 03"; 

// Mapeamento de UIDs para Nomes dos Donos.
// Adicione mais cartões aqui, se necessário.
const String cardOwners[][2] = {
    {"0A C6 3A 03", "Joao"},
    {"F3 BB 22 39", "Maria"},
    {"12 EF AB AB", "Pedro"},
    {"C4 01 0A 01", "Ana"},
};
const int numCards = sizeof(cardOwners) / sizeof(cardOwners[0]);

String UIDCard = "";

// Verifique o endereco do seu display LCD. A maioria e 0x27 ou 0x3F.
LiquidCrystal_I2C lcd(0x3F, 16, 2);

Servo servo;

#define BlueLED 2
#define GreenLED 3
#define RedLED 4
#define Buzzer 5

String getOwnerName(String uid) {
  for (int i = 0; i < numCards; i++) {
    if (cardOwners[i][0] == uid) {
      return cardOwners[i][1];
    }
  }
  return "Desconhecido";
}

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  lcd.init();
  lcd.backlight();
  lcd.clear();
  servo.attach(6);
  servo.write(10);
  pinMode(GreenLED, OUTPUT);
  pinMode(BlueLED, OUTPUT);
  pinMode(RedLED, OUTPUT);
  pinMode(Buzzer, OUTPUT);
  digitalWrite(BlueLED, HIGH);
  lcd.clear();
  lcd.print(" Access Control ");
  lcd.setCursor(0, 1);
  lcd.print("Scan Your Card>>");
}

void loop() {
  digitalWrite(BlueLED, HIGH);
  digitalWrite(RedLED, LOW);
  digitalWrite(GreenLED, LOW);
  noTone(Buzzer);
  servo.write(10);

  if (getUID()) {
    String owner = getOwnerName(UIDCard);
    
    // Imprime o log de forma estruturada para o Monitor Serial
    if (UIDCard == MasterTag) {
        Serial.print("LOG,");
        Serial.print(UIDCard);
        Serial.print(",");
        Serial.print(owner);
        Serial.println(",Acesso Concedido");

        lcd.clear();
        lcd.setCursor(2, 0);
        lcd.print("Permission");
        lcd.setCursor(0, 1);
        lcd.print(" Access Granted!");

        digitalWrite(GreenLED, HIGH);
        digitalWrite(BlueLED, LOW);
        digitalWrite(RedLED, LOW);
        servo.write(100);
        delay(50);
        for (int i = 0; i < 2; i++) {
            tone(Buzzer, 2000);
            delay(250);
            noTone(Buzzer);
            delay(250);
        }
    } else {
        Serial.print("LOG,");
        Serial.print(UIDCard);
        Serial.print(",");
        Serial.print(owner);
        Serial.println(",Acesso Negado");

        lcd.clear();
        lcd.setCursor(2, 0);
        lcd.print("Permission");
        lcd.setCursor(0, 1);
        lcd.print(" Access Denied!");
        
        digitalWrite(BlueLED, LOW);
        digitalWrite(GreenLED, LOW);
        tone(Buzzer, 2000);
        servo.write(10);
        for(int i = 0; i < 10;i++){
            digitalWrite(RedLED, HIGH);
            delay(250);
            digitalWrite(RedLED, LOW);
            delay(250);
        }
        noTone(Buzzer);
    }
    delay(2000);
    lcd.clear();
    lcd.print(" Access Control ");
    lcd.setCursor(0, 1);
    lcd.print("Scan Your Card>>");
  }
}

boolean getUID() {
  if (! mfrc522.PICC_IsNewCardPresent()) {
    return false;
  }
  if (! mfrc522.PICC_ReadCardSerial()) {
    return false;
  }
  UIDCard = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    UIDCard.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    UIDCard.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  UIDCard.toUpperCase();
  UIDCard = UIDCard.substring(1);
  mfrc522.PICC_HaltA();
  return true;
}
