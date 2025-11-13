#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 10
#define SAMPLE 10

const double R = 3.0; // Битовая скорость 
const double T = 1.0/R; // Длительность бита
const double fsym = 1.0/T; // Символьная частота
const double fs = SAMPLE * fsym; // Частота дискретизации
const double Ts = 1.0/fs; // Период дискретизации
const double pi = 3.14;
const int Ns = SIZE * SAMPLE;

typedef struct {
    int I[SIZE];
    int Q[SIZE];
} IQComponents;

// Функция времени
double* create_time_array() {
    double *t = (double*)malloc(Ns * sizeof(double));
    if (t == NULL) {
        return NULL;
    }
    
    for (int i = 0; i < Ns; i++) {
        t[i] = i * Ts;
    }
    
    return t;
}

void generateRandomBits(int *array, int size) {
    for(int i = 0; i < size; i++) {
        array[i] = rand() % 2;
    }
}

int* pulseShaping(int *array, int size) {
    int new_size = size * SAMPLE;
    int *result = malloc(new_size * sizeof(int));
    
    for(int i = 0; i < size; i++) {
        for(int j = 0; j < SAMPLE; j++) {
            result[i * SAMPLE + j] = array[i];
        }
    }
    
    return result;
}

int* acp(int *array,int *array1, int size) {
    int *result = (int*)malloc(size * 2 * sizeof(int));

    for(int i = 0; i < size; i+=2) {
        result[i] = (array[i] * 2047) << 4;
        result[i+1] = (array1[i] * 2047) << 4;
    }

    return result;
}

void signal(int *arrayI, int *arrayQ, int size, double *t) {
    double signal[Ns];
    
    int *bigI = pulseShaping(arrayI, SIZE);
    int *bigQ = pulseShaping(arrayQ, SIZE);

    /*
    printf("Generated signal samples:\n");
    for(int i = 0; i < Ns; i++) { // Показываем сэмплы
        signal[i] = bigI[i] * cos(2 * pi * fsym * t[i]) + bigQ[i] * sin(2 * pi * fsym * t[i]);
        printf("t[%d]=%.6f, signal[%d]=%.6f\n", i, t[i], i, signal[i]);
    }
    */

    free(bigI);
    free(bigQ);
}

IQComponents mapper(int *array, int size) {
    IQComponents result;
    for(int i = 0; i < size; i++) {
        if(array[i] == 0) {
            result.I[i] = -1;
            result.Q[i] = 0;
        }
        else {
            result.I[i] = 1;
            result.Q[i] = 0;
        }
    }
    return result;
}

void create_complex_signal(int *arrayI, int *arrayQ, double *t, double *signalI, double *signalQ) {
    int *bigI = pulseShaping(arrayI, SIZE);
    int *bigQ = pulseShaping(arrayQ, SIZE);
    
    for(int i = 0; i < Ns; i++) {
        signalI[i] = bigI[i] * cos(2 * pi * fsym * t[i]);
        signalQ[i] = bigQ[i] * sin(2 * pi * fsym * t[i]);
    }
    
    free(bigI);
    free(bigQ);
}

int main() {
    srand(time(NULL));
    int bits[SIZE];
    
    // Создаем временной массив
    double *t = create_time_array();
    if (t == NULL) {
        printf("Failed to create time array\n");
        return -1;
    }
    
    printf("Signal parameters:\n");
    printf("  Bit rate R = %.1f bps\n", R);
    printf("  Symbol rate = %.1f sps\n", fsym);
    printf("  Sampling rate = %.1f Hz\n", fs);
    printf("  Time array size = %d samples\n", Ns);
    printf("  Total duration = %.6f seconds\n\n", t[Ns - 1]);
    
    generateRandomBits(bits, SIZE);
    printf("Bits: ");
    for(int i = 0; i < SIZE; i++) {
        printf("%d", bits[i]);
    }
    printf("\n");

    IQComponents iq = mapper(bits, SIZE);
    
    
    printf("I components: ");
    for(int i = 0; i < SIZE; i++) {
        printf("%d ", iq.I[i]);
    }
    printf("\n");
    

    int *bigArrayI = pulseShaping(iq.I, SIZE);
    printf("Extended I components: ");
    for(int i = 0; i < Ns; i++){
        printf("%d", bigArrayI[i]);
    }
    printf("\n");
    int *bigArrayQ = pulseShaping(iq.Q, SIZE);

    int *adalmNum = acp(bigArrayI, bigArrayQ, Ns);
    for (int i = 0; i < Ns; i+=2) {
        printf("%d\n", adalmNum[i]);
        printf("%d\n", adalmNum[i+1]);
    }

    printf("Numbers going to adalm pluto: ");
    for(int i = 0; i < Ns; i++){
        printf("%d ", adalmNum[i]);
    }
    printf("\n");

    /*
    printf("Q components: ");
    for(int i = 0; i < SIZE; i++) {
        printf("%d ", iq.Q[i]);
    }
    printf("\n");
    

    int *bigArrayQ = pulseShaping(iq.Q, SIZE);
    printf("Extended Q components: ");
    for(int i = 0; i < SIZE; i++){
        printf("%d", bigArrayQ[i]);
    }
    printf("\n\n");
    /*
    signal(iq.I, iq.Q, SIZE * SAMPLE, t);
    
    printf("\nComplex signal:\n");
    double signalI[Ns];
    double signalQ[Ns];
    create_complex_signal(iq.I, iq.Q, t, signalI, signalQ);
    
    for(int i = 0; i < 10; i++) {
        printf("Sample %d: I=%.6f, Q=%.6f\n", i, signalI[i], signalQ[i]);
    }
    */
    free(t);
    free(bigArrayI);
    // free(bigArrayQ);
    
    return 0;
}