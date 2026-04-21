#include <SoapySDR/Device.h>   // Инициализация устройства
#include <SoapySDR/Formats.h>  // Типы данных, используемых для записи сэмплов
#include <stdio.h>             // printf
#include <stdlib.h>            // free
#include <stdint.h>
#include <complex.h>
#include <time.h>
#include <math.h>
#include "../include/modulations.h"

#define SIZE  192 
#define SAMPLE 10


#include <stdio.h>
#include <stdlib.h>


int main() {
    FILE *file = fopen("rxdata.pcm", "wb");
    FILE *file1 = fopen("txdata.pcm", "wb");
    int matched_len = (SIZE * SAMPLE * 2) + SAMPLE - 1;


        int16_t bits[SIZE];
        generateRandomBits(bits,SIZE);

        IQComponent Mapped;
        BPSK(bits, SIZE, &Mapped);
    
        int16_t *upSampledI = upSampling(Mapped.Im, SIZE, SAMPLE);
        int16_t *upSampledQ = upSampling(Mapped.Qa, SIZE, SAMPLE);
        //printf("\n\n I \n");
        for(int i=0; i < SIZE * SAMPLE; i++) { 
            //printf("%d ", upSampledI[i]);
        }
        //printf("\n\n Q \n");
        for(int i = 0; i < SIZE * SAMPLE; i++) { 
            //printf("%d ", upSampledQ[i]);
        }
        int16_t convI[SIZE*SAMPLE+SAMPLE-1];
        int16_t convQ[SIZE*SAMPLE+SAMPLE-1];

        convolvePulse(upSampledI, SIZE*SAMPLE, pulse_arr, SAMPLE, convI);
        convolvePulse(upSampledQ, SIZE*SAMPLE, pulse_arr, SAMPLE, convQ);
        
        int16_t *arrayForTX = acp(convI, convQ, SIZE*SAMPLE+SAMPLE-1);
        int counter = 0;
        for(int i = 0; i < ((SIZE * SAMPLE) * 2); i++) { 
            printf("%d ", arrayForTX[i]);
            counter += 1;
            //fwrite(&arrayForTX[i], sizeof(int16_t), 1, file1);  
        
    }
    return 0;
}
