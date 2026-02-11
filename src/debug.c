// main.c

#include <stdio.h>
#include <math.h>
#include "../include/functions.h"



int main() {
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