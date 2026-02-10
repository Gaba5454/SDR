#include <SoapySDR/Device.h>   // Инициализация устройства
#include <SoapySDR/Formats.h>  // Типы данных, используемых для записи сэмплов
#include <stdio.h>             // printf
#include <stdlib.h>            // free
#include <stdint.h>
#include <complex.h>
#include <time.h>
#include <math.h>

#define SIZE 192 
#define SAMPLE 10

const double R = 3.0; // Битовая скорость 
const double T = 1.0/R; // Длительность бита
const double fsym = 1.0/T; // Символьная частота
const double fs = SAMPLE * fsym; // Частота дискретизации
const double Ts = 1.0/fs; // Период дискретизации
const double pi = 3.14;
const int Ns = SIZE * SAMPLE; // 1920

int16_t pulse_arr[10] = {1,1,1,1,1,1,1,1,1,1};
typedef struct {
    int Ii[SIZE];
    int Q[SIZE];
} IQComponents;

void generateRandomBits(int *array, int size) {
    for(int i = 0; i < size; i++) {
        array[i] = rand() % 2;
    }
}

int* pulseShaping(int *array, int size) {
    int new_size = size * SAMPLE;
    int *result = malloc(new_size * sizeof(int));
    
    for(int i = 0; i < size;  i++) {
        for(int j = 0; j < SAMPLE; j++) {
            result[i * SAMPLE + j] = array[i] * pulse_arr[j];
        }
    }

    return result;
}

int16_t* matchedFilter(int16_t *signal, int signal_size, int *filter, int filter_size, int *output_size) {
    *output_size = signal_size; // Для простоты оставляем тот же размер
    int16_t *result = malloc(*output_size * sizeof(int16_t));
    
    // Инициализация выходного массива нулями
    for(int i = 0; i < *output_size; i++) {
        result[i] = 0;
    }
    
    // Выполнение свертки с прямоугольным импульсом
    for(int i = 0; i < signal_size - filter_size + 1; i++) {
        for(int j = 0; j < filter_size; j++) {
            result[i + filter_size - 1] += signal[i + j] * filter[j];
        }
    }
    
    return result;
}

int16_t *acp(int *arrayI, int *arrayQ, int size) {
    int16_t *result = (int16_t*)malloc(2 * size * sizeof(int16_t));

    for(int i = 0; i < size; i++) {
        result[2*i] = (arrayI[i] * 1500) << 4;   // I
        result[2*i+1] = (arrayQ[i] * 1500) << 4; // Q
    }

    return result;
}

IQComponents mapper(int *array, int size) {
    IQComponents result;
    for(int i = 0; i < size; i++) {
        if(array[i] == 0) {
            result.Ii[i] = -1;
            result.Q[i] = 0;
        }
        else {
            result.Ii[i] = 1;
            result.Q[i] = 0;
        }
    }
    return result;
}

int main() {
    SoapySDRKwargs args = {};
    SoapySDRKwargs_set(&args, "driver", "plutosdr");        // Говорим какой Тип устройства 
    if (1) {
        SoapySDRKwargs_set(&args, "uri", "usb:");           // Способ обмена сэмплами (USB)
    } else {
        SoapySDRKwargs_set(&args, "uri", "ip:192.168.2.1"); // Или по IP-адресу
    }
    SoapySDRKwargs_set(&args, "direct", "1");               // 
    SoapySDRKwargs_set(&args, "timestamp_every", "1920");   // Размер буфера + временные метки
    SoapySDRKwargs_set(&args, "loopback", "0");             // Используем антенны или нет
    SoapySDRDevice *sdr = SoapySDRDevice_make(&args);       // Инициализация
    SoapySDRKwargs_clear(&args);
    int sample_rate = 1e6;
    int carrier_freq = 800e6;
    // Параметры RX части
    SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_RX, 0, sample_rate);
    SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_RX, 0, carrier_freq , NULL);

    // Параметры TX части
    SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_TX, 0, sample_rate);
    SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_TX, 0, carrier_freq , NULL);
    // Инициализация количества каналов RX\TX (в AdalmPluto он один, нулевой)
    size_t channels[] = {0};

    // Настройки усилителей на RX\TX
    SoapySDRDevice_setGain(sdr, SOAPY_SDR_RX, channels[0], 10.0); // Чувствительность приемника
    SoapySDRDevice_setGain(sdr, SOAPY_SDR_TX, channels[0], -10.0);// Усиление передатчика

    size_t channel_count = sizeof(channels) / sizeof(channels[0]);
    // Формирование потоков для передачи и приема сэмплов
    SoapySDRStream *rxStream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_RX, SOAPY_SDR_CS16, channels, channel_count, NULL);
    SoapySDRStream *txStream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_TX, SOAPY_SDR_CS16, channels, channel_count, NULL);

    SoapySDRDevice_activateStream(sdr, rxStream, 0, 0, 0); //start streaming
    SoapySDRDevice_activateStream(sdr, txStream, 0, 0, 0); //start streaming

    // Получение MTU (Maximum Transmission Unit), в нашем случае - размер буферов. 
    size_t rx_mtu = SoapySDRDevice_getStreamMTU(sdr, rxStream);
    size_t tx_mtu = SoapySDRDevice_getStreamMTU(sdr, txStream);
    int16_t rx_buffer[2*rx_mtu];
    size_t count = 0;

    srand(time(NULL));

    // Генерация данных для передачи (1920 сэмплов)
    int bits[SIZE];
    generateRandomBits(bits, SIZE);

    IQComponents iq = mapper(bits, SIZE);

    int *bigArrayI = pulseShaping(iq.Ii, SIZE);
    int *bigArrayQ = pulseShaping(iq.Q, SIZE);
    
    int16_t *tx_buff = acp(bigArrayI, bigArrayQ, Ns); // Ns = 1920

     for(int i = 0; i < Ns; i++) {
         printf("%d ",tx_buff[i]);
     }
    printf("\n");

    for(size_t i = 0; i < 2; i++) {
        tx_buff[0 + i] = 0xffff;
        // 8 x timestamp words
        tx_buff[10 + i] = 0xffff;
    }

    const long  timeoutUs = 400000;
    long long last_time = 0;
    int iteration_count = 10;
    FILE *file = fopen("rxdata.pcm", "w");
    for (size_t buffers_read = 0; buffers_read < iteration_count; buffers_read++) {
        void *rx_buffs[] = {rx_buffer};
        int flags;
        long long timeNs;
        
        int sr = SoapySDRDevice_readStream(sdr, rxStream, rx_buffs, rx_mtu, &flags, &timeNs, timeoutUs);
        fwrite(rx_buffer, 2 * rx_mtu * sizeof(int16_t), 1, file);
        
        printf("Buffer: %zu - Samples: %i, Flags: %i, Time: %lli, TimeDiff: %lli\n", buffers_read, sr, flags, timeNs, timeNs - last_time);
        last_time = timeNs;

        long long tx_time = timeNs + (6 * 1000 * 1000); // на 6 мс в будущее
        for(size_t i = 0; i < 8; i++) {
            uint8_t tx_time_byte = (tx_time >> (i * 8)) & 0xff;
            tx_buff[2 + i] = tx_time_byte << 4;
        }
        void *tx_buffs[] = {tx_buff};
        flags = SOAPY_SDR_HAS_TIME;
        if(buffers_read == 2){
            int st = SoapySDRDevice_writeStream(sdr, txStream, (const void * const*)tx_buffs, Ns, &flags, tx_time, timeoutUs);
            if (st != Ns) {
            printf("TX Failed: %i\n", st);
        }
        if(buffers_read == 6){
        printf("\n");
            for(int i = 0; i < Ns; i++) {
            printf("%d ", rx_buffs[i]);
            }
        }
        
        
        }
    }
    fclose(file);

    SoapySDRDevice_deactivateStream(sdr, rxStream, 0, 0);
    SoapySDRDevice_deactivateStream(sdr, txStream, 0, 0);

    SoapySDRDevice_closeStream(sdr, rxStream);
    SoapySDRDevice_closeStream(sdr, txStream);

    SoapySDRDevice_unmake(sdr);

    free(bigArrayI);
    free(bigArrayQ);
    free(tx_buff);

    return 0;
}