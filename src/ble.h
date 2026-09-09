#ifndef BLE_H
#define BLE_H
#include "main.h"
#include <NimBLEDevice.h>

extern NimBLECharacteristic *pTx;
extern bool dispositivoConectado;

void setupBLE();
#endif
