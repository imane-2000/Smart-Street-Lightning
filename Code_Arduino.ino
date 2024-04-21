#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#define WIFI_SSID "Infinix HOT 10"   // Nom du réseau WiFi
#define WIFI_PASSWORD "0631672222"   // Mot de passe du réseau WiFi

#define MQTT_SERVER "io.adafruit.com"   // Serveur MQTT (Adafruit IO)
#define MQTT_PORT 1883   // Port MQTT
#define MQTT_USERNAME "najouael"    // Nom d'utilisateur MQTT
#define MQTT_PASSWORD "aio_Oblx59AvBXcsqHuPEJ1ioI2oi4zt"    // Mot de passe MQTT

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD);

// Déclaration des objets MQTT
Adafruit_MQTT_Publish ledsAllumeesPublish(&mqtt, MQTT_USERNAME "/feeds/leds-allumees");
Adafruit_MQTT_Publish ledsAtteintPublish(&mqtt, MQTT_USERNAME "/feeds/leds-atteintes");
Adafruit_MQTT_Publish ledsPublish(&mqtt, MQTT_USERNAME "/feeds/LdrDurée");

// Initialisation des broches pour les capteurs et les LED

int ir1 = 13;
int led1 = 14;

int ir2 = 25;
int led2 = 32;

int ldr = 34;

int x1, x2;

// Variables pour suivre l'état des LED

int led1Allumee = 0;
int led2Allumee = 0;
int ledsAllumees = 0;
int ledsAtteint = 0;

void setup() {
  
  Serial.begin(115200);
  delay(10);
  pinMode(ir1, INPUT);
  pinMode(led1, OUTPUT);
  pinMode(ir2, INPUT);
  pinMode(led2, OUTPUT);

  // Connexion Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connexion au Wi-Fi...");
  }
  Serial.println("Connexion Wi-Fi établie!");

  // Connexion MQTT
  mqtt.connect();
  while (!mqtt.connected()) {
    Serial.println("Connexion à MQTT en cours...");
    delay(1000);
  }
  Serial.println("Connecté à MQTT d'Adafruit IO!");
}

// Déclaration des variables globales
int previousLedsAllumees = 0;
int previousLedsAtteint = 0;

void loop() {
  
  // Lecture des états des capteurs

  x1 = digitalRead(ir1);
  x2 = digitalRead(ir2);
  int ldr_Value = analogRead(ldr);
  Serial.println(ldr_Value);

  static int previousLdrValue = 0;
  if (ldr_Value != previousLdrValue) {
    // Publier la valeur du capteur LDR sur Adafruit IO
    ledsPublish.publish(ldr_Value);
    previousLdrValue = ldr_Value;  // Mettre à jour la valeur précédente avec la nouvelle valeur
  }
  
  // Contrôle des LED en fonction des capteurs IR
  if(ldr_Value <= 4095){
    if (x1 == HIGH) {  // pas de Mouvement détecté par le capteur 1
      if (led1Allumee > 0) {
        Serial.print("LED 1 éteinte après : ");
        Serial.print(led1Allumee * 0.3);
        Serial.println(" s");
      }
      digitalWrite(led1, LOW);  // Éteindre la LED 1
      led1Allumee = 0;
      Serial.println("LED 1 atteinte");
    } else {           // mouvement détecté par le capteur 1
      if (led1Allumee == 0) {
        Serial.print("LED 1 allumée à : ");
        Serial.print(millis() * 0.001);
        Serial.println(" s");
        Serial.println("LED 1 allumée");
      }
      digitalWrite(led1, HIGH);   // Allumer la LED 1
      led1Allumee++;
    }
    
    if (x2 == HIGH) {  // pas de Mouvement détecté par le capteur 2
      if (led2Allumee > 0) {
        Serial.print("LED 2 éteinte après : ");
        Serial.print(led2Allumee * 0.3);
        Serial.println(" s");
      }
      digitalWrite(led2, LOW);  // Éteindre la LED 2
      led2Allumee = 0;
      Serial.println("LED 2 atteinte");
    } else {           // mouvement détecté par le capteur 2
      if (led2Allumee == 0) {
        Serial.print("LED 2 allumée à : ");
        Serial.print(millis() * 0.001);
        Serial.println(" s");
        Serial.println("LED 2 allumée");
      }
      digitalWrite(led2, HIGH);   // Allumer la LED 2
      led2Allumee++;
    }
  }else{
    digitalWrite(led1,LOW);
    digitalWrite(led2,LOW);
  }
  
  // Mise à jour du nombre de LED allumées et atteintes
  if (led1Allumee > 0 && led2Allumee > 0) {
    ledsAllumees = 2;
    ledsAtteint = 0;
  } else if (led1Allumee > 0 || led2Allumee > 0) {
    ledsAllumees = 1;
    ledsAtteint = 1;
    
  } else {
    ledsAllumees = 0;
    ledsAtteint = 2;
    
  }
  
  bool ledsAllumeesMessageEnvoye = false; // Variable pour suivre si le message des LEDs allumées a été envoyé
  bool ledsAtteintMessageEnvoye = false; // Variable pour suivre si le message des LEDs atteintes a été envoyé
  // Afficher le nombre de LEDs allumées une seule fois
  if (!ledsAllumeesMessageEnvoye) {
    Serial.print("Nombre de LEDs allumées : ");
    Serial.println(ledsAllumees);
    ledsAllumeesMessageEnvoye = true; // Marquer le message comme envoyé
  }
  // Afficher le nombre de LEDs atteintes une seule fois
  if (!ledsAtteintMessageEnvoye) {
    Serial.print("Nombre de LEDs atteintes : ");
    Serial.println(ledsAtteint);
    ledsAtteintMessageEnvoye = true; // Marquer le message comme envoyé
  }

  // Envoi des résultats à Adafruit MQTT si un changement se produit
  if (ledsAllumees != previousLedsAllumees) {
    ledsAllumeesPublish.publish(ledsAllumees);
    previousLedsAllumees = ledsAllumees;
  }

  if (ledsAtteint != previousLedsAtteint) {
    ledsAtteintPublish.publish(ledsAtteint);
    previousLedsAtteint = ledsAtteint;
  }
  delay(300);
}
