#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>


struct ComandoRobot {
    char tipo;
    int valorEntero;
    float valorFloat;
};

extern QueueSetHandle_t colaComandos;
#endif
