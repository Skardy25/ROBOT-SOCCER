#include "main.h"
#include "ble.h"
#include "motores.h"

// Definimos la cola declarada en main.h
QueueHandle_t colaComandos = NULL;

void setup() {
  Serial.begin(9600);
  delay(200);

  colaComandos = xQueueCreate(10, sizeof(ComandoRobot));

  if(colaComandos == NULL){
    Serial.println("Error critico al crear la cola");
    while (1);
  }

  setupBLE();
  setupMotores();

  xTaskCreatePinnedToCore(
    tareaMotores,  // funcion ubicada en motores.cpp
    "TareaMotores", //nombre interno
    4096, //Stack en bytes
    NULL, //Parametros
    2,    //Prioridad
    NULL, //Handler
    1     //Core 1
  );
}

void loop() {
  if (!dispositivoConectado){
    Serial.println("Esperando conexion BLE...");
  }
  else{
   Serial.println("Conexion exitosa");
  }
  delay(3000);
}
