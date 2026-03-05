// mode.h
/*
#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "modulations.h"


void FullMode(SoapySDRDevice *sdr, int16_t *array){
    int sample_rate = 1e6;
    int carrier_freq = 800e6;
    
    // Параметры RX части
    SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_RX, 0, sample_rate);
    SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_RX, 0, carrier_freq , NULL);

    // Параметры TX части
    SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_TX, 0, sample_rate);
    SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_TX, 0, carrier_freq , NULL);
    // Инициализация количества каналов RX\\TX (в AdalmPluto он один, нулевой)
    size_t channels[] = {0};
    
    // Настройки усилителей на RX\\TX
    SoapySDRDevice_setGain(sdr, SOAPY_SDR_RX, channels[0], 50.0); // Чувствительность приемника
    SoapySDRDevice_setGain(sdr, SOAPY_SDR_TX, channels[0], -20.0);// Усиление передатчика
    
    size_t channel_count = sizeof(channels) / sizeof(channels[0]);
    // Формирование потоков для передачи и приема сэмплов
    SoapySDRStream *rxStream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_RX, SOAPY_SDR_CS16, channels, channel_count, NULL);
    SoapySDRStream *txStream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_TX, SOAPY_SDR_CS16, channels, channel_count, NULL);
    
    SoapySDRDevice_activateStream(sdr, rxStream, 0, 0, 0); //start streaming
    SoapySDRDevice_activateStream(sdr, txStream, 0, 0, 0); //start streaming
    //
    // Получение MTU (Maximum Transmission Unit), в нашем случае - размер буферов. 
    const size_t rx_mtu = SoapySDRDevice_getStreamMTU(sdr, rxStream);
    size_t tx_mtu = SoapySDRDevice_getStreamMTU(sdr, txStream);
    // int16_t tx_buff[2*tx_mtu]; // Original
    int16_t rx_buffer[2*rx_mtu];
    size_t count = 0;
    int16_t *tx_buff = array;
    FILE *file1 = fopen("../build/txdata.pcm", "wb");
    fwrite(tx_buff, 2* tx_mtu * sizeof(int16_t), 1, file1);
    fclose(file1);
  
    //prepare fixed bytes in transmit buffer
    //we transmit a pattern of FFFF FFFF [TS_0]00 [TS_1]00 [TS_2]00 [TS_3]00 [TS_4]00 [TS_5]00 [TS_6]00 [TS_7]00 FFFF FFFF
    //that is a flag (FFFF FFFF) followed by the 64 bit timestamp, split into 8 bytes and packed into the lsb of each of the DAC words.
    //DAC samples are left aligned 12-bits, so each byte is left shifted into place
    for(size_t i = 0; i < 2; i++) {
        tx_buff[0 + i] = 0xffff;
        // 8 x timestamp words
        tx_buff[10 + i] = 0xffff;
    }

    const long  timeoutUs = 400000;
    long long last_time = 0;

    // Количество итерация чтения из буфера
    size_t iteration_count = 20;

    int matched_len = (SIZE * SAMPLE) + SAMPLE - 1;
    // Начинается работа с получением и отправкой сэмплов
    FILE *file = fopen("../build/rxdata.pcm", "wb");

    for (size_t buffers_read = 0; buffers_read < iteration_count; buffers_read++) {
        
        void *rx_buffs[] = {rx_buffer};
        int flags;        // flags set by receive operation
        long long timeNs; //timestamp for receive buffer
        
        // считали буффер RX, записали его в rx_buffer
        int sr = SoapySDRDevice_readStream(sdr, rxStream, rx_buffs, rx_mtu, &flags, &timeNs, timeoutUs);

        int16_t *deinterleaved_i = malloc(SIZE * SAMPLE * sizeof(int16_t));
        int16_t *deinterleaved_q = malloc(SIZE * SAMPLE * sizeof(int16_t));

        for (int idx = 0; idx < SIZE * SAMPLE; idx++) {
            deinterleaved_i[idx] = rx_buffer[2 * idx];     // I: 0, 2, 4...
            deinterleaved_q[idx] = rx_buffer[2 * idx + 1]; // Q: 1, 3, 5...
        }

        int32_t *out_i = calloc(matched_len, sizeof(int32_t));
        int32_t *out_q = calloc(matched_len, sizeof(int32_t));
        
        printf("sr=%d, first 5 rx_buffer: %d %d %d %d %d\n", 
       sr, rx_buffer[0], rx_buffer[1], rx_buffer[2], rx_buffer[3], rx_buffer[4]);
        printf("first 5 deinterleaved_i: %d %d %d %d %d\n",
       deinterleaved_i[0], deinterleaved_i[1], deinterleaved_i[2], deinterleaved_i[3], deinterleaved_i[4]);
        matchedFilter(deinterleaved_i, (SIZE * SAMPLE), pulse_arr, SAMPLE, out_i);
        matchedFilter(deinterleaved_q, (SIZE * SAMPLE), pulse_arr, SAMPLE, out_q);
        // Размер после децимации: один отсчёт на символ
int decimated_size = SIZE;  // 192 символа
int16_t *decim_i = malloc(decimated_size * sizeof(int16_t));
int16_t *decim_q = malloc(decimated_size * sizeof(int16_t));

// Оптимальная точка выборки: середина символа (SAMPLE/2) + задержка фильтра
int offset = 1;  // 5 для SAMPLE=10

for (int sym = 0; sym < SIZE; sym++) {
    // Индекс в matched filter output: начало символа + смещение
    int idx = sym * SAMPLE + offset;
    
    // Проверка границ
    if (idx < matched_len) {
        decim_i[sym] = (int16_t)(out_i[idx] / 10);  // Масштабирование
        decim_q[sym] = (int16_t)(out_q[idx] / 10);
    } else {
        decim_i[sym] = 0;  // Заполнение нулями при выходе за границы
        decim_q[sym] = 0;
    }
}

for (int i = 0; i < decimated_size; i++) {
    fwrite(&decim_i[i], sizeof(int16_t), 1, file);
    fwrite(&decim_q[i], sizeof(int16_t), 1, file);
}

free(decim_i);
free(decim_q);

        /*for(int i = 0; i < matched_len; i++){
                int16_t out_i_16 = (int16_t)(out_i[i]);  // Масштабирование
                int16_t out_q_16 = (int16_t)(out_q[i]);
                fwrite(&out_i_16, sizeof(int16_t), 1, file);
                fwrite(&out_q_16, sizeof(int16_t), 1, file);
        }
        free(deinterleaved_i);
        free(deinterleaved_q);*/

        /*for(int i = 0; i < rx_mtu*2; i++){
        if(rx_buffer[i] > 500 || rx_buffer[i] < -500) {
            fwrite(&rx_buffer[i], sizeof(int16_t), 1, file);
            }
        }*/
/*
        // Смотрим на количество считаных сэмплов, времени прихода и разницы во времени с чтением прошлого буфера
        printf("Buffer: %lu - Samples: %i, Flags: %i, Time: %lli, TimeDiff: %lli\n", buffers_read, sr, flags, timeNs, timeNs - last_time);
        last_time = timeNs;

        // Переменная для времени отправки сэмплов относительно текущего приема
        long long tx_time = timeNs + (6 * 1000 * 1000); // на 4 [мс] в будущее
        // Добавляем время, когда нужно передать блок tx_buff, через tx_time -наносекунд
        
        // Здесь отправляем наш tx_buff массив
        void *tx_buffs[] = {tx_buff};
        printf("%d\n", 1920 * buffers_read * 2);
        
        for(size_t i = 0; i < 8; i++) {
            uint8_t tx_time_byte = (tx_time >> (i * 8)) & 0xff;
            tx_buff[2 + i] = tx_time_byte << 4;
        }
            printf("buffers_read: %d\n", buffers_read);
            flags = SOAPY_SDR_HAS_TIME;
            int st = SoapySDRDevice_writeStream(sdr, txStream, (const void * const*)tx_buffs, tx_mtu, &flags, tx_time, timeoutUs);
    }
    fclose(file);
    //stop streaming
    SoapySDRDevice_deactivateStream(sdr, rxStream, 0, 0);
    SoapySDRDevice_deactivateStream(sdr, txStream, 0, 0);

    //shutdown the stream
    SoapySDRDevice_closeStream(sdr, rxStream);
    SoapySDRDevice_closeStream(sdr, txStream);

    //cleanup device handle
    SoapySDRDevice_unmake(sdr);
}
*/
// mode.h

#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "modulations.h"

// ============================================================================
// Структура для комплексных чисел (если понадобится в будущем)
// ============================================================================
typedef struct {
    float re;
    float im;
} complex_float;


void FullMode(SoapySDRDevice *sdr, int16_t *tx_array) {
    
    // === Настройка SDR ===
    int sample_rate = 1e6;
    int carrier_freq = 800e6;
    
    SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_RX, 0, sample_rate);
    SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_RX, 0, carrier_freq, NULL);
    SoapySDRDevice_setSampleRate(sdr, SOAPY_SDR_TX, 0, sample_rate);
    SoapySDRDevice_setFrequency(sdr, SOAPY_SDR_TX, 0, carrier_freq, NULL);
    
    size_t channels[] = {0};
    SoapySDRDevice_setGain(sdr, SOAPY_SDR_RX, channels[0], 40.0);  // Усиление RX
    SoapySDRDevice_setGain(sdr, SOAPY_SDR_TX, channels[0], -20.0); // Усиление TX
    
    size_t channel_count = sizeof(channels) / sizeof(channels[0]);
    SoapySDRStream *rxStream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_RX, SOAPY_SDR_CS16, channels, channel_count, NULL);
    SoapySDRStream *txStream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_TX, SOAPY_SDR_CS16, channels, channel_count, NULL);
    
    SoapySDRDevice_activateStream(sdr, rxStream, 0, 0, 0);
    SoapySDRDevice_activateStream(sdr, txStream, 0, 0, 0);
    
    // === Получение размеров буферов ===
    const size_t rx_mtu = SoapySDRDevice_getStreamMTU(sdr, rxStream);
    const size_t tx_mtu = SoapySDRDevice_getStreamMTU(sdr, txStream);
    printf("\ntx_mtu = %zu\nrx_mtu = %zu\n", tx_mtu, rx_mtu);
    
    int32_t rx_buffer[2 * rx_mtu];  // Interleaved: [I0, Q0, I1, Q1, ...]
    int16_t *tx_buff = tx_array;
    
    // Сохранение TX-данных для отладки
    FILE *filetx = fopen("../build/txdata.pcm", "wb");
    fwrite(tx_buff, 2 * tx_mtu * sizeof(int16_t), 1, filetx);
    fclose(filetx);
    
    // Подготовка преамбулы для TX
    /*for(size_t i = 0; i < 2; i++) {
        tx_buff[0 + i] = 0xffff;
        tx_buff[10 + i] = 0xffff;
    }*/
    
    const long timeoutUs = 400000;
    long long last_time = 0;
    size_t iteration_count = 20;
    
    // Файлы для RX-данных
    FILE *filerx = fopen("../build/rxdata.pcm", "wb");
    FILE *filerx_raw = fopen("../build/raw_rxdata.pcm", "wb");
    
    // Цикл обработки
    for (size_t buffers_read = 0; buffers_read < iteration_count; buffers_read++) {
        
        void *rx_buffs[] = {rx_buffer};
        
        
        int flags;
        long long timeNs;
        
        // 1. Чтение из SDR
        int sr = SoapySDRDevice_readStream(sdr, rxStream, rx_buffs, rx_mtu, &flags, &timeNs, timeoutUs);
        fwrite(rx_buffer, sizeof(int16_t), 2 * sr, filerx_raw);
        
        int16_t *rxDataI = calloc(sr, sizeof(int16_t));
        int16_t *rxDataQ = calloc(sr, sizeof(int16_t));
        for(int i = 0; i < sr; i++){
            rxDataI[i] = rx_buffer[i*2];
            rxDataQ[i] = rx_buffer[i*2+1];
        }

        // 2. Matched Filter
        int matched_len = sr + SAMPLE - 1;
        int32_t *matchRxDataI32 = calloc(matched_len, sizeof(int32_t));
        int32_t *matchRxDataQ32 = calloc(matched_len, sizeof(int32_t));
        convolveMatched(rxDataI, sr, pulse_arr, SAMPLE, matchRxDataI32);
        convolveMatched(rxDataQ, sr, pulse_arr, SAMPLE, matchRxDataQ32);

        int16_t scale = 1000;

        for(int i = 0; i < matched_len; i++){
            int16_t matchRxDataI16 = (int16_t)(matchRxDataI32[i] / scale);
            fwrite(&matchRxDataI16, sizeof(int16_t), 1, filerx);
            int16_t matchRxDataQ16 = (int16_t)(matchRxDataQ32[i] / scale);
            fwrite(&matchRxDataQ16, sizeof(int16_t), 1, filerx);
        }
        /*
        // 3. TED: Поиск оптимальных точек выборки
        int *optimal_indices = malloc(SIZE * sizeof(int));
        
        int found_symbols = findOptimalOffset(rxData, matched_len, SAMPLE, SIZE, optimal_indices);
        
        
        /* 7. Запись в файл
        for (int i = 0; i < found_symbols; i++) {
            fwrite(&decoded_i[i], sizeof(int16_t), 1, file);
            fwrite(&decoded_q[i], sizeof(int16_t), 1, file);
        }*/
        
        
        //Очистка памяти
        
        
        //Лог времени
        printf("Buffer: %lu - TimeDiff: %lli ns\n\n", buffers_read, timeNs - last_time);
        last_time = timeNs;
        
        //Отправка TX
        long long tx_time = timeNs + (6 * 1000 * 1000);
        void *tx_buffs[] = {tx_buff};
        
        for(size_t i = 0; i < 8; i++) {
            uint8_t tx_time_byte = (tx_time >> (i * 8)) & 0xff;
            tx_buff[2 + i] = tx_time_byte << 4;
        }
        
        flags = SOAPY_SDR_HAS_TIME;
        int st = SoapySDRDevice_writeStream(sdr, txStream, (const void * const*)tx_buffs, 
                                           tx_mtu, &flags, tx_time, timeoutUs);
        if (st < 0) {
            printf("TX error: %d\n", st);
        }
        free(rxDataI);
        free(rxDataQ);
        free(matchRxDataI32);
        free(matchRxDataQ32);
    }
    fclose(filerx);
    fclose(filerx_raw);
    
    // === Очистка SDR ===
    SoapySDRDevice_deactivateStream(sdr, rxStream, 0, 0);
    SoapySDRDevice_deactivateStream(sdr, txStream, 0, 0);
    SoapySDRDevice_closeStream(sdr, rxStream);
    SoapySDRDevice_closeStream(sdr, txStream);
    SoapySDRDevice_unmake(sdr);
    
    printf("FullMode завершена успешно\n");
}