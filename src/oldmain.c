#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <complex.h>
#include <time.h>
#include <math.h>

#define SIZE 192 
#define SAMPLE 10




int16_t barker[7] = {1,1,1,-1,-1,1,-1};

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
    
    for(int i = 0; i < size; i++) {
        for(int j = 0; j < SAMPLE; j++) {
            result[i * SAMPLE + j] = array[i] * pulse_arr[j];
        }
    }
    return result;
}

int* matchedFilter(int *array, int size) {
    int new_size = size - SAMPLE + 1;
    if (new_size <= 0) {
        int *result = malloc(sizeof(int));
        result[0] = 0;
        return result;
    }
    
    int *result = malloc(new_size * sizeof(int));
    
    for(int i = 0; i < new_size; i++) {
        result[i] = 0;
        for(int j = 0; j < SAMPLE; j++) {
            result[i] += array[i + j] * pulse_arr[j];
        }
    }
    return result;
}

int16_t *acp(int *arrayI, int *arrayQ, int size) {
    int16_t *result = (int16_t*)malloc(2 * size * sizeof(int16_t));
    for(int i = 0; i <= 6; i++){
        result[i] = barker[i];
    }
    for(int i = 7; i < size; i++) {
        result[2*i] = (arrayI[i] * 1500) << 4;
        result[2*i+1] = 0;
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
/*
// Функция обрезки шума - ДОБАВЛЕНО
// int16_t* trimNoise(int16_t *buffer, int samples, int *new_samples) {
//     if (samples < 10) { // Слишком мало сэмплов
//         *new_samples = 0;
//         return NULL;
//     }
    
//     // Ищем начало сигнала
//     int start = -1;
//     for (int i = 0; i < samples/2 - 5; i += 2) {
//         // Проверяем несколько последовательных сэмплов
//         int has_signal = 1;
//         for (int j = 0; j < 5; j++) {
//             int idx = i + j*2;
//             if (abs(buffer[idx]) < 200 && abs(buffer[idx+1]) < 200) {
//                 has_signal = 0;
//                 break;
//             }
//         }
//         if (has_signal) {
//             start = (i > 20) ? i - 20 : 0;
//             break;
//         }
//     }
    
//     if (start == -1) {
//         *new_samples = 0;
//         return NULL;
//     }
    
//     // Ищем конец сигнала
//     int end = -1;
//     for (int i = samples/2 - 2; i >= start; i -= 2) {
//         int has_signal = 1;
//         for (int j = 0; j < 5; j++) {
//             int idx = i - j*2;
//             if (idx >= 0) {
//                 if (abs(buffer[idx]) < 200 && abs(buffer[idx+1]) < 200) {
//                     has_signal = 0;
//                     break;
//                 }
//             }
//         }
//         if (has_signal) {
//             end = (i + 20 < samples/2) ? i + 20 : samples/2;
//             break;
//         }
//     }
    
//     if (end == -1 || end <= start) {
//         *new_samples = 0;
//         return NULL;
//     }
    
//     // Выделяем память для обрезанных данных
//     int total_samples = (end - start) * 2;
//     int16_t *trimmed = (int16_t*)malloc(total_samples * sizeof(int16_t));
    
//     // Копируем только полезную часть
//     for (int i = 0; i < total_samples/2; i++) {
//         int src_idx = (start + i) * 2;
//         trimmed[2*i] = buffer[src_idx];
//         trimmed[2*i+1] = buffer[src_idx + 1];
//     }
    
//     *new_samples = total_samples/2;
//     return trimmed;
// }
*/

void TXOnlyMode(SoapySDRDevice *sdr) {
    
    int sample_rate = 1e6;
    int carrier_freq = 800e6;
    SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_TX, 0, sample_rate);
    SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_TX, 0, carrier_freq , NULL);

    size_t channels[] = {0};
    SoapySDRDevice_setGain(sdr, SOAPY_SDR_RX, channels[0], 0.0);
    SoapySDRDevice_setGain(sdr, SOAPY_SDR_TX, channels[0], -40.0);

    size_t channel_count = sizeof(channels) / sizeof(channels[0]);
    SoapySDRStream *txStream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_TX, SOAPY_SDR_CS16, channels, channel_count, NULL);
    
    SoapySDRDevice_activateStream(sdr, txStream, 0, 0, 0);
    size_t tx_mtu = SoapySDRDevice_getStreamMTU(sdr, txStream);

    srand(time(NULL));

    int bits[SIZE];                     
    generateRandomBits(bits, SIZE);
    
    IQComponents iq = mapper(bits, SIZE); 

    int *bigArrayI = pulseShaping(iq.Ii, SIZE); 
    
    int *bigArrayQ = pulseShaping(iq.Q, SIZE);  
    
    int16_t *tx_buff = acp(bigArrayI, bigArrayQ, Ns); 

/* Отладка
    // Генерация случайных битов
    printf("\nGengeration bits...\n");
        for(int i = 0; i < SIZE; i++){
            printf("%d ",bits[i]);
        }
    // Пробрасываем биты через маппер
    printf("\nBits to IQ components...\n");
    for(int i = 0; i < SIZE; i++){
        printf("%d ",iq.Ii[i]);
        printf("%d ",iq.Q[i]);
    }
    // Делаем свертку для I компонент
    printf("\nI components...\n");
    for(int i = 0; i < SIZE; i++){
        printf("%d ",bigArrayI[i]);
    }
    // Делаем свертку для Q компонент    
    printf("\nQ components...\n");
    for(int i = 0; i < SIZE; i++){
        printf("%d ",bigArrayQ[i]);
    }
    // Получаем итоговый буфер    
    printf("\nComplete buffer...\n");
    for(int i = 0; i < Ns; i++){
        printf("%d ",tx_buff[i]);
    }
    printf("\n");
*/

    const long timeoutUs = 400000;
    long long last_time = 0;
    int iteration_count = 1;
    FILE *file = fopen("txdata.pcm", "wb");
    for (size_t buffers_read = 0; buffers_read < iteration_count; buffers_read++) {
        int flags;
        long long timeNs = 0;

        long long tx_time = timeNs + (6 * 1000 * 1000);
        for(size_t i = 0; i < 8; i++) {
            uint8_t tx_time_byte = (tx_time >> (i * 8)) & 0xff;
            tx_buff[2 + i] = tx_time_byte << 4;
        }
        void *tx_buffs[] = {tx_buff};
        flags = SOAPY_SDR_HAS_TIME;
        
        int st = SoapySDRDevice_writeStream(sdr, txStream, (const void * const*)tx_buffs, Ns, &flags, tx_time, timeoutUs);
        fwrite(tx_buff, 2 * tx_mtu * sizeof(int16_t), 1, file);
    }

    SoapySDRDevice_deactivateStream(sdr, txStream, 0, 0);
    SoapySDRDevice_closeStream(sdr, txStream);
    free(bigArrayI);
    free(bigArrayQ);
    free(tx_buff);
}

void RXOnlyMode(SoapySDRDevice *sdr) {
    int sample_rate = 1e6;
    int carrier_freq = 800e6;

    SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_RX, 0, sample_rate);
    SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_RX, 0, carrier_freq , NULL);

    size_t channels[] = {0};
    SoapySDRDevice_setGain(sdr, SOAPY_SDR_RX, channels[0], 10.0);
    SoapySDRDevice_setGain(sdr, SOAPY_SDR_TX, channels[0], 0.0);

    size_t channel_count = sizeof(channels) / sizeof(channels[0]);
    SoapySDRStream *rxStream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_RX, SOAPY_SDR_CS16, channels, channel_count, NULL);

    SoapySDRDevice_activateStream(sdr, rxStream, 0, 0, 0);
    size_t rx_mtu = SoapySDRDevice_getStreamMTU(sdr, rxStream);
    int16_t rx_buffer[2*rx_mtu];

    srand(time(NULL));

    const long timeoutUs = 400000;
    long long last_time = 0;
    int iteration_count = 10000;
    FILE *file = fopen("rxdata.pcm", "wb");
    int switcher = 1;
    int buffer = 0;
    while (switcher == 1) {
        void *rx_buffs[] = {rx_buffer};
        int flags;
        long long timeNs;

        int sr = SoapySDRDevice_readStream(sdr, rxStream, rx_buffs, rx_mtu, &flags, &timeNs, timeoutUs);
        fwrite(rx_buffer, 2 * rx_mtu * sizeof(int16_t), 1, file);

        printf("Buffer: %zu - Samples: %i, Flags: %i, Time: %lli, TimeDiff: %lli\n", 
                ++buffer, sr, flags, timeNs, timeNs - last_time);
    }
    fclose(file);

    SoapySDRDevice_deactivateStream(sdr, rxStream, 0, 0);
    SoapySDRDevice_closeStream(sdr, rxStream);
}

void FullMode(SoapySDRDevice *sdr){
    int sample_rate = 1e6;
    int carrier_freq = 800e6;
    
    SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_RX, 0, sample_rate);
    SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_RX, 0, carrier_freq , NULL);
    SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_TX, 0, sample_rate);
    SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_TX, 0, carrier_freq , NULL);
    
    size_t channels[] = {0};
    SoapySDRDevice_setGain(sdr, SOAPY_SDR_RX, channels[0], 50.0);
    SoapySDRDevice_setGain(sdr, SOAPY_SDR_TX, channels[0], -10.0);

    size_t channel_count = sizeof(channels) / sizeof(channels[0]);
    SoapySDRStream *rxStream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_RX, SOAPY_SDR_CS16, channels, channel_count, NULL);
    SoapySDRStream *txStream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_TX, SOAPY_SDR_CS16, channels, channel_count, NULL);

    SoapySDRDevice_activateStream(sdr, rxStream, 0, 0, 0);
    SoapySDRDevice_activateStream(sdr, txStream, 0, 0, 0);

    size_t rx_mtu = SoapySDRDevice_getStreamMTU(sdr, rxStream);
    size_t tx_mtu = SoapySDRDevice_getStreamMTU(sdr, txStream);
    
    srand(time(NULL));

    int bits[SIZE];
    generateRandomBits(bits, SIZE);
    IQComponents iq = mapper(bits, SIZE);
    int *bigArrayI = pulseShaping(iq.Ii, SIZE);
    int *bigArrayQ = pulseShaping(iq.Q, SIZE);
    int16_t *tx_buff = acp(bigArrayI, bigArrayQ, Ns);

    const long timeoutUs = 400000;
    long long last_time = 0;
    int iteration_count = 10;
    
    // Буфер для хранения всех принятых данных
    size_t total_rx_samples = 0;
    size_t max_rx_samples = iteration_count * rx_mtu;
    int16_t *all_rx_data = malloc(2 * max_rx_samples * sizeof(int16_t));
    
    // Переменные для передачи
    int tx_sent = 0; // Флаг, что передача выполнена
    
    for (size_t buffers_read = 0; buffers_read < iteration_count; buffers_read++) {
        // Прием данных
        int16_t rx_buffer[2 * rx_mtu];
        void *rx_buffs[] = {rx_buffer};
        int flags;
        long long timeNs;
        
        int sr = SoapySDRDevice_readStream(sdr, rxStream, rx_buffs, rx_mtu, &flags, &timeNs, timeoutUs);
        
        // Сохраняем принятые данные во временный буфер
        if (sr > 0) {
            memcpy(all_rx_data + total_rx_samples * 2, rx_buffer, sr * 2 * sizeof(int16_t));
            total_rx_samples += sr;
        }
        
        
        long long tx_time = timeNs + (6 * 1000 * 1000);
            
        for(size_t i = 0; i < 8; i++) {
            uint8_t tx_time_byte = (tx_time >> (i * 8)) & 0xff;
            tx_buff[2 + i] = tx_time_byte << 4;
        }
            
        void *tx_buffs[] = {tx_buff};
        flags = SOAPY_SDR_HAS_TIME;
        int st = SoapySDRDevice_writeStream(sdr, txStream, (const void * const*)tx_buffs, tx_mtu, &flags, tx_time, timeoutUs);
            
        // Сохраняем переданные данные в файл
        FILE *filetx = fopen("txdata.pcm", "wb");
        fwrite(tx_buff, 2 * tx_mtu * sizeof(int16_t), 1, filetx);
        fclose(filetx);
    
        
        last_time = timeNs;
            
    }

    // Обработка принятых данных: обрезка шума
    // Создаем буфер для очищенных данных
    int16_t *clean_rx_data = malloc(2 * total_rx_samples * sizeof(int16_t));
    size_t clean_samples = 0;
    
    // Пороговое значение для шума
    const int16_t NOISE_THRESHOLD = 1000;
    
    // Фильтрация: оставляем только сэмплы с амплитудой выше порога
    for (size_t i = 0; i < total_rx_samples; i++) {
        int16_t Ii = all_rx_data[2 * i];
        int16_t Q = all_rx_data[2 * i + 1];
        
        // Проверяем, не является ли это шумом (амплитуда в диапазоне [-200, 200])
        if (abs(Ii) > NOISE_THRESHOLD || abs(Q) > NOISE_THRESHOLD) {
            clean_rx_data[2 * clean_samples] = Ii;
            clean_rx_data[2 * clean_samples + 1] = Q;
            clean_samples++;
        }
    }
    
    // Извлекаем I-компоненту из очищенных данных для применения согласованного фильтра
    int *I_components = malloc(clean_samples * sizeof(int));
    for (size_t i = 0; i < clean_samples; i++) {
        I_components[i] = clean_rx_data[2 * i];
    }
    
    // Применяем согласованный фильтр к I-компоненте
    int *filtered_I = matchedFilter(I_components, clean_samples);
    int filtered_size = clean_samples - SAMPLE + 1;
    
    // Преобразуем результат фильтрации в формат I/Q (Q=0)
    int16_t *filtered_complex = malloc(2 * filtered_size * sizeof(int16_t));
    for (int i = 0; i < filtered_size; i++) {
        // Масштабируем результат, чтобы он поместился в int16_t
        int32_t scaled = filtered_I[i];
        
        // Проверяем и ограничиваем значения
        if (scaled > 32767) scaled = 32767;
        if (scaled < -32768) scaled = -32768;
        
        filtered_complex[2 * i] = (int16_t)scaled;  // I-компонента
        filtered_complex[2 * i + 1] = 0;            // Q-компонента
    }
    
    // Сохраняем СВЕРНУТЫЕ данные в файл rxdata.pcm
    FILE *filerx = fopen("rxdata.pcm", "wb");
    if (filtered_size > 0) {
        fwrite(filtered_complex, 2 * sizeof(int16_t), filtered_size, filerx);
    }
    fclose(filerx);
    
    // Выводим информацию о количестве записанных сэмплов
    printf("Записано %d свернутых сэмплов в rxdata.pcm\n", filtered_size);
    
    // Освобождаем память
    free(all_rx_data);
    free(clean_rx_data);
    free(I_components);
    free(filtered_I);
    free(filtered_complex);
    free(bigArrayI);
    free(bigArrayQ);
    free(tx_buff);

    SoapySDRDevice_deactivateStream(sdr, rxStream, 0, 0);
    SoapySDRDevice_deactivateStream(sdr, txStream, 0, 0);
    SoapySDRDevice_closeStream(sdr, rxStream);
    SoapySDRDevice_closeStream(sdr, txStream);
}

int main(int argc, char *argv[]) {

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

    if (choose == 1) {
        RXOnlyMode(sdr);
    } else if (choose == 2) {
        TXOnlyMode(sdr);
    } else if (choose == 3) {
        FullMode(sdr);
    } else {
        fprintf(stderr, "Invalid mode selected.\n");
    }
    SoapySDRDevice_unmake(sdr);
    return 0;
}
/*
    // int sample_rate = 1e6;
    // int carrier_freq = 800e6;
    
    // SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_RX, 0, sample_rate);
    // SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_RX, 0, carrier_freq , NULL);
    // SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_TX, 0, sample_rate);//
    // SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_TX, 0, carrier_freq , NULL);//
    
    // size_t channels[] = {0};
    // SoapySDRDevice_setGain(sdr, SOAPY_SDR_RX, channels[0], 10.0);
    // SoapySDRDevice_setGain(sdr, SOAPY_SDR_TX, channels[0], -10.0);

    // size_t channel_count = sizeof(channels) / sizeof(channels[0]);
    // SoapySDRStream *rxStream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_RX, SOAPY_SDR_CS16, channels, channel_count, NULL);
    // SoapySDRStream *txStream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_TX, SOAPY_SDR_CS16, channels, channel_count, NULL);

    // SoapySDRDevice_activateStream(sdr, rxStream, 0, 0, 0);
    // SoapySDRDevice_activateStream(sdr, txStream, 0, 0, 0);

    // size_t rx_mtu = SoapySDRDevice_getStreamMTU(sdr, rxStream);
    // size_t tx_mtu = SoapySDRDevice_getStreamMTU(sdr, txStream);
    // int16_t rx_buffer[2*rx_mtu];
    
    // srand(time(NULL));

    // int bits[SIZE];
    // generateRandomBits(bits, SIZE);
    // IQComponents iq = mapper(bits, SIZE);
    // int *bigArrayI = pulseShaping(iq.Ii, SIZE);
    // int *bigArrayQ = pulseShaping(iq.Q, SIZE);
    // int16_t *tx_buff = acp(bigArrayI, bigArrayQ, Ns);

    // const long timeoutUs = 400000;
    // long long last_time = 0;
    // int iteration_count = 10;
    // FILE *file = fopen("rxdata.pcm", "wb");
    
    // for (size_t buffers_read = 0; buffers_read < iteration_count; buffers_read++) {
    //     void *rx_buffs[] = {rx_buffer};
    //     int flags;
    //     long long timeNs;

    //     int sr = SoapySDRDevice_readStream(sdr, rxStream, rx_buffs, rx_mtu, &flags, &timeNs, timeoutUs);
        
    //     if (sr > 0) {
    //         int trimmed_samples;
    //         int16_t *trimmed_data = trimNoise(rx_buffer, 2*sr, &trimmed_samples);
            
    //         if (trimmed_data != NULL && trimmed_samples > 0) {
    //             fwrite(trimmed_data, 2 * trimmed_samples * sizeof(int16_t), 1, file);
    //             free(trimmed_data);
    //         }
    //     }
        
    //     printf("Buffer: %zu - Samples: %i, Flags: %i, Time: %lli, TimeDiff: %lli\n", 
    //            buffers_read, sr, flags, timeNs, timeNs - last_time);
    //     last_time = timeNs;

    //     long long tx_time = timeNs + (6 * 1000 * 1000);
    //     for(size_t i = 0; i < 8; i++) {
    //         uint8_t tx_time_byte = (tx_time >> (i * 8)) & 0xff;
    //         tx_buff[2 + i] = tx_time_byte << 4;
    //     }
    //     void *tx_buffs[] = {tx_buff};
    //     flags = SOAPY_SDR_HAS_TIME;
        
    //     int st = SoapySDRDevice_writeStream(sdr, txStream, (const void * const*)tx_buffs, Ns, &flags, tx_time, timeoutUs);
          
    // }
    // fclose(file);

    // SoapySDRDevice_deactivateStream(sdr, rxStream, 0, 0);
    // SoapySDRDevice_deactivateStream(sdr, txStream, 0, 0);
    // SoapySDRDevice_closeStream(sdr, rxStream);
    // SoapySDRDevice_closeStream(sdr, txStream);
    // SoapySDRDevice_unmake(sdr);

    // free(bigArrayI);
    // free(bigArrayQ);
    // free(tx_buff);

    // return 0;
*/