// ----- OPENCLIGNOTANT ---------------- //
// Vélomix 2024 -- Rennes           ---- //
// Baptiste Gautier et Tony Vanpoucke -- //

#include <M5StickC.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN 26   // Pin de sortie pour la LED ring
#define NUM_LEDS 12  // Nombre de LEDs dans la LED ring

// ---- CONFIGURATION DU CLIGNOTANT ---- //
// - changer les variables pour config - //
 
int sensibilite = 6;                 // Sensibilité d'activation du clignotant, plus c'est fort plus c'est dur de l'activer.
int tempo = 80;                      // Le tempo de clignotement, plus c'est fort cela clignotte lentement.
int veille = 17;                     // plus c'est fort et moins la led brille durant le mode veille uniquement.
int repete = 9;                      // nombre de clingotement par séquence de clignotant.
unsigned long eventThreshold = 200;  // delais dans lequel la secousse doit être repérée pour s'activer.
const int numReadings = 10;          // Lissage du signal, joue sur la sensibilité de captation.

int rouge = 0;                       // couleur du clignotant 255 = 100% et 0 = 0%
int vert = 0;
int bleu = 255;

// ------------------------------------ //
// ------------------------------------ //

// Initialisation de la bande de LEDs NeoPixel
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Variables pour les gyros
float gyroX, gyroY, gyroZ;
float accX, accY, accZ;

float readings[numReadings];  // readings for accelerometer data
int readIndex = 0;            // the index of the current reading
int total = 0;                // the running total
int average = 0;              // the average
int bounce = 0;
int lumVeille = 0;

// Variables pour le suivi des événements
unsigned long lastNegativeEvent = 0;
unsigned long lastPositiveEvent = 0;
int negativeCount = 0;
int positiveCount = 0;

void setup() {
  // Initialisation de M5StickC et du capteur MPU6886
  M5.begin();
  M5.MPU6886.Init();
  M5.Axp.begin();  // Initialiser le gestionnaire d'alimentation AXP192

  // Couper l'alimentation des périphériques inutiles
  M5.Axp.ScreenBreath(0);  // Éteindre l'écran

  Serial.begin(115200);

  // Initialiser la bande NeoPixel
  strip.begin();
  strip.show();             // Éteindre toutes les LEDs au départ
  strip.setBrightness(50);  // Optionnel : ajuster la luminosité

  // initialize all the readings to 0:
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }
}

void loop() {
  // M5.update() permet de vérifier les événements sur les boutons
  M5.update();

  // Vérifier l'appui du bouton A
  if (M5.BtnA.wasPressed()) {
    // Alterner la valeur de lumVeille entre 0 et 1
    lumVeille = !lumVeille;
    Serial.print("lumVeille: ");
    Serial.println(lumVeille);
  }

  // Reste du code (gestion des gyros et animations LED)
  M5.MPU6886.getAccelData(&accX, &accY, &accZ);

  // Gestion des lectures pour le calcul de la moyenne
  total = total - readings[readIndex];  // Soustraire l'ancienne valeur
  readings[readIndex] = accX;           // Lire la nouvelle valeur
  total = total + readings[readIndex];  // Ajouter la nouvelle lecture
  readIndex = readIndex + 1;

  if (readIndex >= numReadings) {
    readIndex = 0;  // Réinitialiser l'index si nécessaire
  }

  average = total / numReadings;  // Calculer la moyenne
  Serial.println(average);

  unsigned long currentTime = millis();  // Temps actuel

  // Si la moyenne est négative
  if (average < 0) {
    if (currentTime - lastNegativeEvent <= eventThreshold) {
      negativeCount++;
    } else {
      negativeCount = 1;  // Réinitialiser le compteur si plus de 2 secondes sont passées
    }
    lastNegativeEvent = currentTime;

    if ((negativeCount >= sensibilite) && (bounce == 0)) {
      Serial.println("Action pour 2 valeurs négatives !");
      flashDroit();
      negativeCount = 0;  // Réinitialiser après action
    }
  }

  // Si la moyenne est supérieure ou égale à 1
  if (average >= 1) {
    if (currentTime - lastPositiveEvent <= eventThreshold) {
      positiveCount++;
    } else {
      positiveCount = 1;  // Réinitialiser le compteur si plus de 2 secondes sont passées
    }
    lastPositiveEvent = currentTime;

    if ((positiveCount >= sensibilite) && (bounce == 0)) {
      Serial.println("Action pour 2 valeurs positives !");
      flashGauche();
      positiveCount = 0;  // Réinitialiser après action
    }
  }

  if (average == 0) {
    bounce = 0;
    BougePas();
  }

  delay(1);  // Délai entre les lectures pour stabiliser
}

void flashGauche() {
  if (bounce == 0) {
    for (int x = 0; x <= repete; x++) {
      strip.clear();  // Effacer toutes les LEDs avant de commencer
      strip.show();

      // Animation de la LED de 7 à 12
      for (int i = 6; i <= 12; i++) {                                         // Effacer toutes les LEDs
        strip.setPixelColor(i, strip.Color(rouge, vert, bleu));  // Allumer la LED en rouge
        strip.show();
      }
      delay(tempo);  // Pause pour l'effet d'animation

      // Animation de la LED de 12 à 7
      for (int i = 6; i <= 12; i++) {
        strip.clear();                                           // Effacer toutes les LEDs
        strip.show();
      }
      delay(tempo);  // Pause pour l'effet d'animation

      strip.clear();  // Éteindre toutes les LEDs après l'animation
      strip.show();
    }
    bounce = 1;
  }
}

void flashDroit() {
  if (bounce == 0) {
    for (int x = 0; x <= repete; x++) {
      strip.clear();  // Effacer toutes les LEDs avant de commencer
      strip.show();

      // Animation de la LED de 7 à 12
      for (int i = 0; i <= 6; i++) {                                         // Effacer toutes les LEDs
        strip.setPixelColor(i, strip.Color(rouge, vert, bleu));  // Allumer la LED en rouge
        strip.show();
      }
      delay(tempo);  // Pause pour l'effet d'animation

      // Animation de la LED de 12 à 7
      for (int i = 0; i <= 6; i++) {
        strip.clear();                                           // Effacer toutes les LEDs
        strip.show();
      }
      delay(tempo);  // Pause pour l'effet d'animation

      strip.clear();  // Éteindre toutes les LEDs après l'animation
      strip.show();
    }
    bounce = 1;
  }
}

void BougePas() {
  //truc à animer quand il se passe rien
  if (lumVeille == 1) {
    for (int i = 0; i <= 11; i++) {                                                             // Parcourt les LEDs de 0 à 11
      strip.setPixelColor(i, strip.Color((rouge / veille), (vert / veille), (bleu / veille)));  // Allumer la LED avec les couleurs définies
    }
    strip.show();  // Mettre à jour les LEDs
  } else {
    strip.clear();
    strip.show();
  }
}
