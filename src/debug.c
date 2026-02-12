// main.c

#include <stdio.h>
#include <math.h>
#include "../include/functions.h"



int main() {
    parametr.R = 3.0;
    parametr.T = 1.0;
    parametr.fsym = 1.0/parametr.T;
    parametr.fd = SAMPLE * parametr.fsym;
    parametr.Ts = 1.0/parametr.fd;
    parametr.Ns = SIZE * SAMPLE;

    int bits[SIZE];
    generateRandomBits(bits,SIZE);
    printf("Generated bits:\n");
    for(int i = 0; i < SIZE; i++) {
        printf("%d ", bits[i]);
    }

    IQComponent Mapped = BPSK(bits,SIZE);
    printf("\nMapper works: \n");
    for(int i = 0; i < SIZE; i++) {
        printf("%d %d ", Mapped.Im[i], Mapped.Qa[i]);
    }

    int *pulsedI = pulseShaping(Mapped.Im, SIZE, SAMPLE);
    int *pulsedQ = pulseShaping(Mapped.Qa, SIZE, SAMPLE);
    printf("\nPulse shaping: \n");
    for(int i = 0; i < SIZE * SAMPLE; i++) {
        printf("%d ", pulsedI[i]);
    }
    printf("\n");


    return 0;
}