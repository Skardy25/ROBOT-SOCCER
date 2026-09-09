#include "motores.h"

#define AIN1 18
#define AIN2 19
#define BIN1 25
#define BIN2 26

#define PWM_DER 33
#define CANAL_DER 0

#define PWM_IZQ 27
#define CANAL_IZQ 1
#define STBY 4

const int FRECUENCIA = 20000; // Frecuencia de 20 kHz
const int RESOLUCION = 10;

// Resolución de 8 bits (rango 0-255)
// Resolución de 10 bits (rango 0-1023)
// Resolución de 12 bits (rango 0-4095)
const int VALOR_MAXIMO = (1 << RESOLUCION) - 1; 

//curva de velocidad + deadband
struct  MotorCalib
{
    int min_pwm; 
    int max_pwm; 
    float k; //factor de correcion
};

//modificar si se cambia la resolucion
MotorCalib cal_izq = {50, VALOR_MAXIMO, 1.0f}; 
MotorCalib cal_der = {50, VALOR_MAXIMO, 1.0f}; // Si derecho es 5% mas lento , le metes 1.05

//Clase ubicada en motores.h
Movimiento movimietoActual = ALTO; // ñp que hacen los motores
Movimiento movimientoObjetivo = ALTO; // lo que quiere el usuario

//velcoidades 
int velocidadPWM = 0;
int velocidadObjetivo = 0;
int velocidadMaxima = 1000; //velocidad actual por defecto

void setupMotores() {
    //canal digital de los motores
    ledcSetup(CANAL_DER, FRECUENCIA, RESOLUCION);
    ledcSetup(CANAL_IZQ, FRECUENCIA, RESOLUCION);

    ledcAttachPin(PWM_DER, CANAL_DER);
    ledcAttachPin(PWM_IZQ, CANAL_IZQ);

    pinMode(AIN1, OUTPUT);
    pinMode(AIN2, OUTPUT);
    pinMode(BIN1, OUTPUT);
    pinMode(BIN2, OUTPUT);
    pinMode(STBY, OUTPUT);

    //STBY 1 por defecto
    digitalWrite(STBY, HIGH);
}

int aplicarCurva(int pwm_in ,MotorCalib &motor){
    if(pwm_in == 0) return 0;
    pwm_in = constrain(pwm_in, 0, VALOR_MAXIMO);

    int pwm_out = map(pwm_in, 0, VALOR_MAXIMO, motor.min_pwm, motor.max_pwm);
    int output = constrain(pwm_out * motor.k , 0, motor.max_pwm);
    return output;
}

void adelante(int velociadBase){
    int pwm_izq = aplicarCurva(velociadBase, cal_izq);
    int pwm_der = aplicarCurva(velociadBase, cal_der);

    digitalWrite(AIN1, HIGH);digitalWrite(AIN2, LOW);
    ledcWrite(CANAL_IZQ, pwm_izq);
    digitalWrite(BIN1, HIGH);digitalWrite(BIN2, LOW);
    ledcWrite(CANAL_DER, pwm_der);
}

void atras(int velocidad){
    int p_izq = aplicarCurva(velocidad, cal_izq);
    int p_der = aplicarCurva(velocidad, cal_der);
    digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH);
    ledcWrite(CANAL_IZQ, p_izq);
    digitalWrite(BIN1, LOW); digitalWrite(BIN2, HIGH);
    ledcWrite(CANAL_DER, p_der);
}

void giroIzquierda(int velocidad ){
    int p_izq = aplicarCurva(velocidad, cal_izq);
    int p_der = aplicarCurva(velocidad, cal_der);
    digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH);
    ledcWrite(CANAL_IZQ, p_izq);
    digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW);
    ledcWrite(CANAL_DER, p_der);
}

void giroDerecha(int velocidad){
    int p_izq = aplicarCurva(velocidad, cal_izq);
    int p_der = aplicarCurva(velocidad, cal_der);
    digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW);
    ledcWrite(CANAL_IZQ, p_izq);
    digitalWrite(BIN1, LOW); digitalWrite(BIN2, HIGH);
    ledcWrite(CANAL_DER, p_der);
}

void detener(){
    digitalWrite(AIN1, HIGH); digitalWrite(AIN2, HIGH);
    ledcWrite(CANAL_IZQ, 0);
    digitalWrite(BIN1, HIGH); digitalWrite(BIN2, HIGH);
    ledcWrite(CANAL_DER, 0);

}

void actualizarRampa() {
    //Eleva el valor si quieres aumentar velocidad de aceleracion 
    const int paso = 40;

    if(velocidadPWM < velocidadObjetivo){
        velocidadPWM += paso;

        if (velocidadPWM > velocidadObjetivo){
            velocidadPWM = velocidadObjetivo;
        }
        
    }else if(velocidadPWM > velocidadObjetivo){
        velocidadPWM -= paso;
        if(velocidadPWM < velocidadObjetivo){
            velocidadPWM = velocidadObjetivo;
        }
    }

}

void actualizarMotores(){
    switch (movimietoActual)
    {
    case ADELANTE:
        adelante(velocidadPWM);
        break;
    case ATRAS: 
        atras(velocidadPWM);
        break;
    case GIRO_IZQ:
        giroIzquierda(velocidadPWM);
        break;
    case GIRO_DER:
        giroDerecha(velocidadPWM);
        break;
    case ALTO:
        detener();
        break;
    default:
        detener();
        break;
    }
}

void gestionarCambio(){
    if(movimietoActual != movimientoObjetivo){
        velocidadObjetivo = 0;

        if(velocidadPWM == 0){
            movimietoActual = movimientoObjetivo;
            
            if(movimietoActual == ALTO){
                velocidadObjetivo = 0;
            }else{
                velocidadObjetivo = velocidadMaxima;
            }
        }
    }else{
        if(movimietoActual == ALTO){
            velocidadObjetivo = 0;
        }else {
            velocidadObjetivo = velocidadMaxima;
        }
    }
}

void tareaMotores(void *pvParameters){
    ComandoRobot cmd;

    while (true){
        //xQueueReceive(colaComandos, &cmd, portMAX_DELAY)  comando que espera hasta recibir nuevo dato
        if(xQueueReceive(colaComandos, &cmd, pdMS_TO_TICKS(5)) == pdTRUE){
            switch (cmd.tipo){

                case 'F':  
                    Serial.println("[Core 1] Adelante");
                    movimientoObjetivo = ADELANTE;
                    velocidadObjetivo = velocidadMaxima;
                break;
                case 'B':  
                    Serial.println("[Core 1] atras"); 
                    movimientoObjetivo = ATRAS;
                    velocidadObjetivo = velocidadMaxima;
                    break;
                case 'L':  
                    Serial.println("[Core 1] atras"); 
                    movimientoObjetivo = GIRO_IZQ;
                    velocidadObjetivo = velocidadMaxima;
                    break;
                 case 'R':  
                    Serial.println("[Core 1] atras"); 
                    movimientoObjetivo = GIRO_DER;
                    velocidadObjetivo = velocidadMaxima;
                    break;
                case 'G':  
                    Serial.println("[Core 1] atras"); 
                    movimientoObjetivo = ADEL_IZQ;
                    velocidadObjetivo = velocidadMaxima;
                    break;
                case 'I':  
                    Serial.println("[Core 1] atras"); 
                    movimientoObjetivo = ADEL_DER;
                    velocidadObjetivo = velocidadMaxima;
                    break;
                case 'H':  
                    Serial.println("[Core 1] atras"); 
                    movimientoObjetivo = ATRA_IZQ;
                    velocidadObjetivo = velocidadMaxima;
                    break;
                case 'J':  
                    Serial.println("[Core 1] atras"); 
                    movimientoObjetivo = ATRA_DER;
                    velocidadObjetivo = velocidadMaxima;
                    break;
                case 'S':
                    movimientoObjetivo = ALTO;
                    break;
                case 'a': 
                    cal_izq.max_pwm = cmd.valorEntero;
                    break;
                case 'b':
                    cal_izq.k = cmd.valorFloat;
                    break;
                case 'c': 
                    cal_der.max_pwm = cmd.valorEntero;
                    break;
                case 'd':
                    cal_der.k = cmd.valorFloat;
                    break;
                default:
                    Serial.println("[Core 1] Comando de parada");
                    movimientoObjetivo = ALTO;
                    break;
            }
        }
        gestionarCambio();
        actualizarRampa();
        actualizarMotores();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
