#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <time.h>

#define HV 999999
//----------DEFINICIONES----------------//

typedef enum {LLEGADA, SALIDA_A, SALIDA_B} TipoEvento;

typedef struct{
    float tiempo;
    TipoEvento tipo;
    int indice;
} Evento;

typedef struct{
    float TPLL;
    float TPSA;
    float *TPSB;
}TEF;

//----------VARIABLES-----------------//

int CP;

float TF;
float t;

int NSA, NSB;

int NTA, NTB;
int ARR;
float STPA, STPB;
float STAA, STAB;
float STOA, *STOB;
float ITOA, *ITOB;

float PTOA;
float *PTOB;
float PPA;
float PPS, PEC;
int ultimoPuestoBUsado = -1;
//-----------FUNCIONES AUXILIARES-------------//
void iniciarGeneradorRandom(){
    srand(time(NULL));
}

float random2(){
    return (float)rand() / RAND_MAX;
}

int puestoBVacio(TEF *tef){
    int i = (ultimoPuestoBUsado + 1) % CP;
    for(int vuelta = 0; vuelta < CP; vuelta++, i++){
        if(tef->TPSB[i] == HV){
            ultimoPuestoBUsado = i;
            return i;
        }
    }

    return -1;
}

int proximaSalidaB(TEF *tef){
    int ret = 0;
    for(int i = 0; i < CP; i++){
        if(tef->TPSB[i] < tef->TPSB[ret])
            ret = i;
    }
    return ret;
}

int esDomingo(){
    return (int)t % (24 * 7 * 60) < 24;
}

float conseguirIntervaloArribo(){
    if(!esDomingo())
        return (-log(-log(random2())) * 34.502 + 88.855);
    else
        return ((pow(-log(random2()), -0.10992) -1) / 0.10992)*20.772 + 44.683;
}

float conseguirTiempoAtencion(){
    double r = random2();
    while(r == 1) r = random2();
    return (30.0/pow(1.0-r, 1.0/2.1654));
}

//-----------FUNCIONES PRINCIPALES------------//

void establecerCondicionesIniciales(int cantidadPuestos, TEF *tef){
    CP = cantidadPuestos;
    
    t = 0;
    TF = 365 * 24 * 60;
    
    NSA = 0;
    NSB = 0;
    
    NTA = 0, NTB = 0;
    ARR = 0;
    STPA = 0, STPB = 0;
    STAA = 0, STAB = 0;
    STOA = 0;
    STOB = malloc(sizeof(float) * CP);
    for(int i = 0; i < CP; i++) STOB[i] = 0;
    ITOA = 0;
    ITOB = malloc(sizeof(float) * CP);
    for(int i = 0; i < CP; i++) ITOB[i] = 0;
    
    tef->TPLL = 0;
    tef->TPSA = HV;
    
    tef->TPSB = malloc(sizeof(float) * CP);
    for(int i = 0; i < CP; i++)
        tef->TPSB[i] = HV;

    PTOB = malloc(sizeof(float) * CP);
}

Evento definirSiguienteEvento(TEF *tef){
    Evento ret;
    int indiceProximaSalidaB = proximaSalidaB(tef);
    if(tef->TPLL < tef->TPSA && tef->TPLL < tef->TPSB[indiceProximaSalidaB]){
        ret.tipo = LLEGADA;
        ret.tiempo = tef->TPLL;
    }
    else{
        if(tef->TPSA < tef->TPSB[indiceProximaSalidaB]){
            ret.tipo = SALIDA_A;
            ret.tiempo = tef->TPSA;
        }
        else{
            ret.tipo = SALIDA_B;
            ret.tiempo = tef->TPSB[indiceProximaSalidaB];
            ret.indice = indiceProximaSalidaB;
        }
    }

    return ret;
}
void actualizarEFNC(Evento e, TEF *tef){
    if(e.tipo == LLEGADA){
        float IA = conseguirIntervaloArribo(t);
        tef->TPLL = t + IA;
    }
}

int actualizarVectorEstado(Evento *e, TEF *tef){
    switch(e->tipo){
    case LLEGADA: ;
        //Procesar tipo cliente
        float r_tipo_cliente = random2();

        //procesar arrepentimiento
        if(r_tipo_cliente <= 0.3){ // cliente frecuente
            e->indice = 0;
            if(NSA <= 3){
                NSA++;
            }
            else if(NSA == 4){
                float r_arrepentimiento = random2();
                if(r_arrepentimiento <= 0.5)
                    NSA++;
                else{
                    ARR++;
                    return 0;
                }
            }
            else if(NSA == 5){
                float r_arrepentimiento = random2();
                if(r_arrepentimiento <= 0.25) 
                    NSA++;
                else{
                    ARR++;
                    return 0;
                }
            }
            else{
                ARR++;
                return 0;
            }
            NTA++;
        }
        else { // cliente regular
            e->indice = 1;
            if(NSB <= 2 + CP){
                NSB++;
            }
            else if(NSB <= 3 + CP){
                float r_arrepentimiento = random2();
                if(r_arrepentimiento <= 0.5)
                    NSB++;
                else{
                    ARR++;
                    return 0;
                }
            }
            else if(NSB <= 4 + CP){
                float r_arrepentimiento = random2();
                if(r_arrepentimiento <= 0.25) 
                    NSB++;
                else{
                    ARR++;
                    return 0;
                }
            }
            else{
                ARR++;
                return 0;
            }
            NTB++;
        }
        break;
    case SALIDA_A:
        NSA--;
        break;
    case SALIDA_B:
        NSB--;
        break;
    default:
        break;
    }
    return 1;
}

void actualizarEFC(Evento e, TEF *tef){
    switch(e.tipo){
    case LLEGADA:
        if(e.indice == 0){ //--> cliente frecuente
            if(NSA == 1){
                float TA = conseguirTiempoAtencion();
                tef->TPSA = t + TA;
                STAA = STAA + TA;
                STOA = STOA + (t - ITOA);
            }
            else if(NSA == 2 && puestoBVacio(tef) != -1){
                int i = puestoBVacio(tef);
                NSA--;
                NSB++;
                float TA = conseguirTiempoAtencion();
                tef->TPSB[i] = t + TA;
                STAB = STAB + TA;
                STOB[i] = STOB[i] + (t - ITOB[i]);
            }
        }
        else{ //--> cliente regular
            if(NSB <= CP){
                int i = puestoBVacio(tef);
                float TA = conseguirTiempoAtencion();
                tef->TPSB[i] = t + TA;
                STAB = STAB + TA;
                STOB[i] = STOB[i] + (t - ITOB[i]);
            }
        }
        break;
    case SALIDA_A:
        if(NSA >= 1){
            float TA = conseguirTiempoAtencion();
            tef->TPSA = t + TA;
            STAA = STAA + TA;
        }
        else{
            tef->TPSA = HV;
            ITOA = t;
        }
        break;
    case SALIDA_B:
        if(NSA >= 2){
            NSA--;
            NSB++;
            float TA = conseguirTiempoAtencion();
            tef->TPSB[e.indice] = t + TA;
            STAB = STAB + TA;
        }
        else if(NSA <= 2 && NSB >= CP){
            float TA = conseguirTiempoAtencion();
            tef->TPSB[e.indice] = t + TA;
            STAB = STAB + TA;
        }
        else{
            tef->TPSB[e.indice] = HV;
            ITOB[e.indice] = t;
        }
        break;
    default:
        break;
    }
}

void calcularResultados(){
    //Promedio permanencia sistema
    PPS = (STPA + STPB) / (NTA + NTB);

    //Prmedio espera cola
    PEC = (STPA + STPB - (STAA + STAB)) / (NTA + NTB);

    printf("NTA %d\n", NTA);
    printf("NTB %d\n", NTB);
    printf("STAA: %f\n", STAA);
    printf("STAB: %f\n", STAB);
    printf("STPA: %f\n", STPA);
    printf("STPB: %f\n", STPB);
    
    //Porcentaje arrepentidos
    PPA = ARR * 100 / (NTA + NTB + ARR);
    
    //Porcentaje tiempo ocioso
    PTOA = STOA * 100 / t;
    for(int i = 0; i < CP; i++){
        PTOB[i] = STOB[i] * 100 / t;
    }
}
void imprimirResultados(){
    printf("Promedio permanencia sistema: %f \n", PPS);
    printf("Prmedio espera cola: %f \n", PEC);
    printf("Porcentaje arrepentidos %f \n", PPA);
    printf("Porcentaje tiempo ocioso puesto frecuente: %f \n", PTOA);
    printf("Porcentaje tiempo ocioso puesto regular: \n");
    for(int i = 0; i < CP; i++)
        printf("\t Puesto numero %d, tiempo ocioso: %f\n", i, PTOB[i]);
    
    printf("Variable de control (Cantidad puesto): %d\n", CP);
}

int main()
{
    int cantidadPuestos;
    printf("Ingresar cantidad puestos: ");
    scanf("%d",&cantidadPuestos);
    iniciarGeneradorRandom();
    
    printf("Estableciendo condiciones iniciales\n");
    TEF *tef = malloc(sizeof(TEF));
    
    //NOTA: el numero pasado por parametro es la variable de control
    establecerCondicionesIniciales(cantidadPuestos, tef);

    Evento e;
    while(t < TF){
        printf("Eligiendo siguiente evento\n");
        e = definirSiguienteEvento(tef);
        STPA = STPA + (e.tiempo - t) * NSA;
        STPB = STPB + (e.tiempo - t) * NSB;
        printf("Actualizando t\n");
        t = e.tiempo;
        printf("Actualizando EFNC\n");
        actualizarEFNC(e, tef);
        printf("Actualizando EFC\n");
        if(actualizarVectorEstado(&e, tef))
            actualizarEFC(e, tef);
    }
    
    calcularResultados();
    imprimirResultados();

    return 0;
}
