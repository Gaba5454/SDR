// main.c

#include "../include/mode.h"


int main(int argc, char *argv[]) {
    parametr.R = 3.0;
    parametr.T = 1.0;
    parametr.fsym = 1.0/parametr.T;
    parametr.fd = SAMPLE * parametr.fsym;
    parametr.Ts = 1.0/parametr.fd;
    parametr.Ns = SIZE * SAMPLE;

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

    // Преобразование битов в символы
    IQComponent Mapped;
    BPSK(bits, SIZE, &Mapped);
    
    // Апсемплинг
    int16_t *upSampledI = upSampling(Mapped.Im, SIZE, SAMPLE);
    int16_t *upSampledQ = upSampling(Mapped.Qa, SIZE, SAMPLE);

    // Свертка с импульсной характеристикой
    int16_t convI[SIZE*SAMPLE+SAMPLE-1];
    int16_t convQ[SIZE*SAMPLE+SAMPLE-1];
    convolvePulse(upSampledI, SIZE*SAMPLE, pulse_arr, SAMPLE, convI);
    convolvePulse(upSampledQ, SIZE*SAMPLE, pulse_arr, SAMPLE, convQ);

    // Формирование массива на передачу
    int16_t *arrayForTX = acp(convI, convQ, SIZE * SAMPLE);

    /*printf("Choose mode: \n 1 - RX\n 2 - TX\n 3 - FullMode\n");
    int choose = 0;
    scanf("%d", &choose);*/

    FullMode(sdr, arrayForTX);
    free(arrayForTX);
    free(upSampledI);
    free(upSampledQ);
    return 0;
}