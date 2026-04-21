// main.c

#include "../include/mode.h"


int main(int argc, char *argv[]) {
    srand(time(NULL));  
    const char *device_uri = argv[1];
    
    SoapySDRKwargs args = {};
    SoapySDRKwargs_set(&args, "driver", "plutosdr");
    SoapySDRKwargs_set(&args, "uri", device_uri);
    SoapySDRKwargs_set(&args, "direct", "1");
    SoapySDRKwargs_set(&args, "timestamp_every", "1920");
    SoapySDRKwargs_set(&args, "loopback", "0");
    SoapySDRDevice *sdr = SoapySDRDevice_make(&args);
    SoapySDRKwargs_clear(&args);
    
    // Генерация битов
    int16_t bits[SIZE];
    generateRandomBits(bits,SIZE);
    int16_t barkBits[SIZE+BARKER_LEN];
    int idx;
    for (int i=0; i < SIZE+BARKER_LEN; i++){
        idx = i - BARKER_LEN;
        if(i < BARKER_LEN){
            barkBits[i] = BARKER_13[i];
        }
        else {
            barkBits[i] = bits[idx];
        }
    }
    printf("\n");
    printf("bits:");
    for(int i=0; i < SIZE+BARKER_LEN; i++) {
        printf("%d ", barkBits[i]);
    }
    printf("\n");

    
    // Преобразование битов в символы
    IQComponent Mapped;
    BPSK(barkBits, SIZE+BARKER_LEN, &Mapped);
    printf("\n");
    printf("BPSK:");
    printf("\n");
    for(int i=0; i < SIZE+BARKER_LEN; i++) {
        printf("%d ", Mapped.Im[i]);
    }
    printf("\n");
    // Апсемплинг
    int16_t *upSampledI = upSampling(Mapped.Im, SIZE+BARKER_LEN, SAMPLE);
    printf("\n");
    printf("UpSampled:");
    printf("\n");
    for(int i=0; i < (SIZE+BARKER_LEN)*SAMPLE; i++) {
        printf("%d ", upSampledI[i]);
    }
    printf("\n");
    int16_t *upSampledQ = upSampling(Mapped.Qa, SIZE+BARKER_LEN, SAMPLE);

    // Свертка с импульсной характеристикой
    int16_t convI[(SIZE+BARKER_LEN)*SAMPLE];
    int16_t convQ[(SIZE+BARKER_LEN)*SAMPLE];
    convolvePulse(upSampledI, (SIZE+BARKER_LEN)*SAMPLE, pulse_arr, SAMPLE, convI);
    convolvePulse(upSampledQ, (SIZE+BARKER_LEN)*SAMPLE, pulse_arr, SAMPLE, convQ);
    printf("\n");
    printf("ConvI:");
    printf("\n");
    for(int i=0; i < (SIZE+BARKER_LEN)*SAMPLE; i++) {
        printf("%d ", convI[i]);
    }
    printf("\n");
    printf("\n");
    printf("ConvQ:");
    printf("\n");
    for(int i=0; i < (SIZE+BARKER_LEN)*SAMPLE; i++) {
        printf("%d ", convQ[i]);
    }
    printf("\n");
    // Формирование массива на передачу
    int16_t *arrayForTX = acp(convI, convQ, (SIZE+BARKER_LEN) * SAMPLE);
    printf("\n");
    printf("ConvI:");
    printf("\n");
    for(int i=0; i < (SIZE+BARKER_LEN)*SAMPLE*2; i++) {
        printf("%d ", arrayForTX[i]);
    }
    printf("\n");
    /*printf("Choose mode: \n 1 - RX\n 2 - TX\n 3 - FullMode\n");
    int choose = 0;
    scanf("%d", &choose);*/

    FullMode(sdr, arrayForTX);
    int16_t *tx_with_barker = malloc((SIZE + BARKER_LEN) * sizeof(int16_t));
for(int i = 0; i < BARKER_LEN; i++) tx_with_barker[i] = BARKER_13[i];  // +1/-1
for(int i = 0; i < SIZE; i++) tx_with_barker[BARKER_LEN + i] = (bits[i] == 0) ? -1 : 1;

    demapper_and_compare(
        "../build/cor_rxdata.pcm",  // файл с принятыми данными
        tx_with_barker + BARKER_LEN,                     // массив переданных битов
        192                          // количество символов
        );

    free(arrayForTX);
    free(upSampledI);
    free(upSampledQ);
    return 0;
}