#ifndef MOTORES_H
#define MOTORES_H

#include "main.h"
 
enum Movimiento {
    ALTO,
    ADELANTE,
    ATRAS,
    GIRO_DER,
    GIRO_IZQ,
    ADEL_DER,
    ADEL_IZQ,
    ATRA_DER,
    ATRA_IZQ
};

void tareaMotores(void *pvParameters);
void setupMotores();

#endif
