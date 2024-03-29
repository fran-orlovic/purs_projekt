#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <MFRC522.h>
#include <SPI.h>

#define RESET 22
#define SDA 5
#define SCK 18
#define MOSI 23
#define MISO 19

#define GREEN_LED 12
#define RED_LED 13
#define BUZZER 27


MFRC522 rfid(SDA, RESET);

const char* SSID = "HUAWEI P30";
const char* PASSWORD = "12345678";
const char *serverName = "http://192.168.43.244/provjera";

HTTPClient http;
StaticJsonDocument<200> doc;

String provjera(byte uid_byte[], int uid_size);

void setup() {
    Serial.begin(9600);
    SPI.begin();
    rfid.PCD_Init(); // Inicijalizacija MFRC522

    // Spajanje na WiFi
    WiFi.begin(SSID, PASSWORD);
    Serial.print("Connecting to WiFi ..");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print('.');
        delay(500);
    }
    Serial.println(WiFi.localIP());

    pinMode(GREEN_LED, OUTPUT);
    pinMode(RED_LED, OUTPUT);
    pinMode(BUZZER, OUTPUT);
}

void loop() {
    // Resetiraj petlju ako kartica nije u blizini čitača. Stavlja proces u stanje mirovanja.
    if ( ! rfid.PICC_IsNewCardPresent())
        return;
        
    // Provjerava je li NUID pročitan
    if (rfid.PICC_ReadCardSerial()) {
        String rezultat = provjera(rfid.uid.uidByte, rfid.uid.size);
        Serial.print("UID: ");
        Serial.println(rezultat);

        // Spremanje UID-a kao json i slanje
        doc["uid"] = rezultat;
        doc["vrata"] = 2; // Vrata 1-Antolic && Vrata2-Orlovic

        String json;
        serializeJson(doc, json);
        http.begin(serverName);
        http.addHeader("Content-Type", "application/json");
        int httpResponseCode = http.POST(json);

        Serial.print("Status code: ");
        Serial.println(httpResponseCode);

        // Request prošao
        if (httpResponseCode == 201) {
            // Čitanje payloada
            String responsePayload = http.getString();
            Serial.println(responsePayload);

            // Provjera sadrži li payload "Dozvoljeno"
            bool dozvola = responsePayload.indexOf("Dozvoljeno") != -1;

            if (dozvola == true) {
                digitalWrite(GREEN_LED, HIGH);
                tone(BUZZER, 3000);
                delay(1500);
                noTone(BUZZER);
                delay(1500);
                digitalWrite(GREEN_LED, LOW);
            }
            else {
                digitalWrite(RED_LED, HIGH);
                for (int i = 0; i < 3; i++) {
                    tone(BUZZER, 300);
                    delay(500);
                    noTone(BUZZER);
                    delay(500);
                }
                digitalWrite(RED_LED, LOW);
            }
        }
        else {
            Serial.println("HTTP POST failed");
        }

        http.end();

    }
    else
        return;
}

// Funkcija za pretvaranje pročitane vrijednosti u string i dodavanje razmaka između dijelova UID-a
String provjera(byte uid_byte[], int uid_size) {
    String id = "";
    for (int i = 0; i < uid_size; i++)
    {
        id += uid_byte[i] < 0x10 ? " 0" : " ";
        id += String(uid_byte[i], HEX);
    }
    id.remove(0, 1); // Brisanje umetnutog prvog razmaka prije UID-a
    return id;
}