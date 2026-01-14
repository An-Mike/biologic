
//include pour le BLE
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

//UUID BLE
#define SERVICE_UUID        "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define CHAR_UUID_DATA      "6e400003-b5a3-f393-e0a9-e50e24dcca9e"

//include pour la mémoire flash
#include "FS.h"
#include "LittleFS.h"
#define COLS 10
#define ROWS 9000
#define FILE_NAME "/tableau.bin"




BLECharacteristic *dataChar;
bool deviceConnected = false;

/* ===== Callbacks ===== */
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    BLEDevice::startAdvertising();
  }
};

/* ===== Envoi BLE fragmenté ===== */
void sendBLE(const char* msg) {
  const uint8_t CHUNK_SIZE = 20;   // BLE standard MTU
  int len = strlen(msg);
  int offset = 0;

  while (offset < len) {
    uint8_t size = min(CHUNK_SIZE, (uint8_t)(len - offset));
    dataChar->setValue((uint8_t*)(msg + offset), size);
    dataChar->notify();
    delay(30);   // très important pour la stabilité
    offset += size;
  }
}


/* ===== Setup ===== */
void setup() {
  Serial.begin(115200);

  // Initialisation LittleFS
  if (!LittleFS.begin(true)) {
    Serial.println("Erreur LittleFS !");
    return;
  }

  Serial.println("LittleFS monté avec succès");

  // Ouvrir le fichier en écriture (écrase s'il existe)
  File file = LittleFS.open(FILE_NAME, FILE_WRITE);
  if (!file) {
    Serial.println("Impossible de créer le fichier");
    return;
  }

  // Initialisation du générateur aléatoire
  randomSeed(esp_random());

  float value;

  //
  struct data {
  uint16_t tap, tmat, tex, hap, hmat, hex, o2, day, month, bat
  };

  // Remplissage du tableau et écriture en flash
  for (int row = 0; row < ROWS; row++) {
    for (int col = 0; col < COLS; col++) {
      value = random(0, 10000) / 100.0f;  // float entre 0.00 et 100.00
      file.write((uint8_t*)&value, sizeof(float));
    }
  }

  file.close();
  Serial.println("Tableau créé et stocké dans LittleFS");

  // Affichage de la taille du fichier
  File check = LittleFS.open(FILE_NAME, FILE_READ);
  Serial.print("Taille du fichier : ");
  Serial.print(check.size());
  Serial.println(" octets");
  check.close();

  BLEDevice::init("BioLogic");

  BLEServer *server = BLEDevice::createServer();
  server->setCallbacks(new MyServerCallbacks());

  BLEService *service = server->createService(SERVICE_UUID);

  dataChar = service->createCharacteristic(
    CHAR_UUID_DATA,
    BLECharacteristic::PROPERTY_NOTIFY
  );

  dataChar->addDescriptor(new BLE2902());

  service->start();

  BLEAdvertising *advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(SERVICE_UUID);
  advertising->start();

  Serial.println("BioLogic BLE prêt");
}

/* ===== Loop ===== */
void loop() {
  if (deviceConnected) {
    //bac d'apport
    float temperatureApp = random(300, 700) / 10.0;   // 30 à 70 °C
    float oxygen      = random(150, 210) / 10.0;   // 15 à 21 %
    float humidityApp       = random(10,1000) / 10.0;    // de 1 à 100 %


    //bac de maturation
    float temperatureMat = random(300, 700) / 10.0;   // 30 à 70 °C
    float humidityMat       = random(10,1000) / 10.0;    // de 1 à 100 %

    //extérieur
    float temperatureExt = random(300, 700) / 10.0;   // 30 à 70 °C
    float humidityExt       = random(10,1000) / 10.0;    // de 1 à 100 %

    //date en millisec
    //unsigned long t   = millis() / 1000;

    char payload[150];
    sprintf(payload,
      "<{\"tap\":%.1f,\"o2ap\":%.1f,\"hap\":%.1f,\"tmat\":%.1f,\"hmat\":%.1f,\"tex\":%.1f,\"hex\":%.1f}>",
      temperatureApp, oxygen, humidityApp, temperatureMat, humidityMat, temperatureExt, humidityExt
    );

    sendBLE(payload); //envoi d'un paquet

    Serial.println(payload);
    delay(500);
  }
}
