// struct.h

#include <stdio.h>
#include <stdint.h>

#define SIZE 192
#define SAMPLE 10
#define PI 3.14
#define BARKER_LEN 13
int16_t pulse_arr[SAMPLE] = {1,1,1,1,1,1,1,1,1,1};
int16_t BARKER_13[BARKER_LEN] = {1, 1, 1, 1, 1, -1, -1, 1, 1, -1, 1, -1, 1};

typedef struct  {
    double R; //= 3.0;
    double T; //= 1.0/R;
    double fsym; //= 1.0/T;
    double fd; //= SAMPLE * fsym;
    double Ts; //= 1.0/fs;
    int Ns; //= SIZE * SAMPLE;
} signalParametrs;
  signalParametrs parametr;

/* Структура с инициализацией массивов I и Q квадратур. */

typedef struct {
    int16_t Im[SIZE];
    int16_t Qa[SIZE];
} IQComponent; 
