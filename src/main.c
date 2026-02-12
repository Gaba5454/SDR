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

    printf("Choose mode: \n 1 - RX\n 2 - TX\n 3 - FullMode\n");
    int choose = 0;
    scanf("%d", &choose);

    int16_t bits[SIZE];
    generateRandomBits(bits,SIZE);

    IQComponent Mapped;
    BPSK(bits,SIZE, &Mapped);

    int16_t *pulsedI = pulseShaping(Mapped.Im, SIZE, SAMPLE);
    int16_t *pulsedQ = pulseShaping(Mapped.Qa, SIZE, SAMPLE);
    int16_t *arrayForTX = acp(pulsedI, pulsedQ, SIZE * SAMPLE);

    FullMode(sdr, arrayForTX);
    return 0;
}