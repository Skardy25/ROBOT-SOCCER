#include "ble.h"

#define PIN_LED 13

// IDDs para comunicacion BLE
#define SERVICE_UUID  "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX  "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX  "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

//Nombre del Bluethooth
#define BLE_NOMBRE "robotSKARDY"
// variables globales para BLE
NimBLECharacteristic *pTx;
bool dispositivoConectado = false; /* estado de conexion BLE */

//SE ejecuta cuando el cliente BLE se conecta o desconecta
class Servidor : public NimBLEServerCallbacks {

  void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
    dispositivoConectado = true;
    digitalWrite(PIN_LED, HIGH);
    Serial.println(">>> Cliente conectado!");
  }
  void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
    dispositivoConectado = false;
    digitalWrite(PIN_LED, LOW);
    Serial.println(">>> Cliente desconectado");
    pServer->startAdvertising();
  }
};

//Se ejecuta cuando se reciben datos RX
class Receptor : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
    std::string rxValue = pCharacteristic->getValue();
    
    while(!rxValue.empty() && (rxValue.back() == '\n' || rxValue.back() == '\r' || rxValue.back() == ' ')){
      rxValue.pop_back();
    }

    String msg = String(rxValue.c_str());
    msg.trim();
    if(msg.length() == 0 ) return;

    ComandoRobot nuevoCmd;
    nuevoCmd.tipo = '\0';
    nuevoCmd.valorEntero = 0;
    nuevoCmd.valorFloat = 0.0f;

    // Comandos de BLE, si deseas agregar uno tambien 
    // agrega en motores.cpp si es necesario
    if(msg.length() == 1){
      char letra = msg.charAt(0);
      nuevoCmd.tipo = letra;
      if(msg == "L"){
        nuevoCmd.tipo = 'L';
      }
      else if(msg == "F"){
        nuevoCmd.tipo = 'F';
      }
      else if(msg == "B"){
        nuevoCmd.tipo = 'B'; 
      }
      else if(msg == "S"){
        nuevoCmd.tipo = 'S'; 
      }
      else if(msg == "R"){
        nuevoCmd.tipo = 'R'; 
      }
      else if(msg == "G"){
        nuevoCmd.tipo = 'G'; 
      }
      else if(msg == "I"){
        nuevoCmd.tipo = 'I'; 
      }
      else if(msg == "H"){
        nuevoCmd.tipo = 'H'; 
      }
      else if(msg == "J"){
        nuevoCmd.tipo = 'J'; 
      }

    }else if(msg.length() > 1){
      //para Motor A
      if(msg.startsWith("a")){
        nuevoCmd.tipo = 'a';
        nuevoCmd.valorEntero = msg.substring(1).toInt();
        Serial.println("Valor leido a: " + String(nuevoCmd.valorEntero));

      }else if(msg.startsWith("b")){
        nuevoCmd.tipo = 'b';
        nuevoCmd.valorFloat = msg.substring(1).toFloat();
        Serial.println("Valor leido b: " + String(nuevoCmd.valorFloat));
        
      }
      //para el Motor B
      else if(msg.startsWith("c")){
        nuevoCmd.tipo = 'c';
        nuevoCmd.valorEntero = msg.substring(1).toInt();
        Serial.println("Valor leido c: " + String(nuevoCmd.valorFloat));
        
      }
      else if(msg.startsWith("d")){
        nuevoCmd.tipo = 'd';
        nuevoCmd.valorFloat = msg.substring(1).toFloat();
        Serial.println("Valor leido c: " + String(nuevoCmd.valorFloat));
      }
    }else{
      return;
    }
    if(colaComandos != NULL){
      xQueueSend(colaComandos, &nuevoCmd, portMAX_DELAY);
    }
  }
};

void setupBLE() {
  
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);


  Serial.println("LED OK");

  NimBLEDevice::init(BLE_NOMBRE);
  NimBLEDevice::setDeviceName(BLE_NOMBRE);
  Serial.println("BLE iniciado");

  NimBLEServer *pServidor = NimBLEDevice::createServer();
  
  pServidor->setCallbacks(new Servidor());

  NimBLEService *pService = pServidor->createService(SERVICE_UUID);

  pTx = pService->createCharacteristic(
    CHARACTERISTIC_UUID_TX,
    NIMBLE_PROPERTY::NOTIFY
  );
  Serial.println("TX OK");

  NimBLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID_RX,
    NIMBLE_PROPERTY::WRITE
  );
  pRxCharacteristic->setCallbacks(new Receptor());

  pService->start();
  Serial.println("Servicio iniciado");

  NimBLEAdvertising *pAnunciar = NimBLEDevice::getAdvertising();
  pAnunciar->setName(BLE_NOMBRE);
  pAnunciar->addServiceUUID(SERVICE_UUID);
  pAnunciar->start();
}
