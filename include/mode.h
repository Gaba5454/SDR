// mode.h

#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "modulations.h"


void FullMode(SoapySDRDevice *sdr, int16_t *tx_array) {
    
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
    
    int16_t rx_buffer[2 * rx_mtu];
    int16_t *tx_buff = tx_array;
    
    // Сохранение TX-данных для отладки
    FILE *filetx = fopen("../build/txdata.pcm", "wb");
    fwrite(tx_buff, 2 * tx_mtu * sizeof(int16_t), 1, filetx);
    fclose(filetx);
    
    const long timeoutUs = 400000;
    long long last_time = 0;
    size_t iteration_count = 10;
    
    // Файлы для RX-данных
    FILE *filerx = fopen("../build/rxdata.pcm", "wb");
    FILE *filerx_raw = fopen("../build/raw_rxdata.pcm", "wb");
    FILE *file_ted = fopen("../build/ted_rxdata.pcm", "wb");


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

        int16_t scale = 100;

        int16_t *matchRxDataI16 = calloc(matched_len, sizeof(int16_t));
        int16_t *matchRxDataQ16 = calloc(matched_len, sizeof(int16_t));
        for(int i = 0; i < matched_len; i++){
            matchRxDataI16[i] = (int16_t)(matchRxDataI32[i] / scale);
            matchRxDataQ16[i] = (int16_t)(matchRxDataQ32[i] / scale);
            fwrite(&matchRxDataI16[i], sizeof(int16_t), 1, filerx);
            fwrite(&matchRxDataQ16[i], sizeof(int16_t), 1, filerx);
        }
        
        // 3.TED
        // В FullMode после matched filter:
        int Nsps = SAMPLE;

        // 1. Находим лучший offset
        int best_offset = find_best_offset(matchRxDataI16, matchRxDataQ16, matched_len, Nsps);
        printf("Best offset: %d\n", best_offset);

        // 2. Записываем символы с этим offset
        int num_symbols = matched_len / Nsps;
        for (int sym = 0; sym < num_symbols; sym++) {
            int idx = sym * Nsps + best_offset;
            if (idx < matched_len) {
                    fwrite(&matchRxDataI16[idx], sizeof(int16_t), 1, file_ted);
                    fwrite(&matchRxDataQ16[idx], sizeof(int16_t), 1, file_ted);
            }
        }


        //Лог времени
        printf("Buffer: %lu - TimeDiff: %lli ns\n\n", buffers_read, timeNs - last_time);
        last_time = timeNs;
        
        //Отправка TX
        long long tx_time = timeNs + (6 * 1000 * 1000);
        void *tx_buffs[] = {tx_buff};
        
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
        free(matchRxDataI16);
        free(matchRxDataQ16);
    }
    fclose(filerx);
    fclose(filerx_raw);
    fclose(file_ted);
    
// === 4. Частотная синхронизация (Costas Loop) ===
// Costas Loop убирает вращение созвездия (частотную ошибку)
costas_loop_sync("../build/ted_rxdata.pcm", "../build/sync_rxdata.pcm", 
                 0.05,   // loop_bw
                 1.0);   // damping

debug_signal("../build/sync_rxdata.pcm", 192);

// === 5. Кадровая синхронизация и финальная коррекция полярности ===
// Эта функция теперь сама проверит инверсию и исправит её при записи
extract_after_barker(
    "../build/sync_rxdata.pcm",    // вход (уже после Costas)
    "../build/cor_rxdata.pcm",     // выход
    192                            // символов
);
    // === Очистка SDR ===
    SoapySDRDevice_deactivateStream(sdr, rxStream, 0, 0);
    SoapySDRDevice_deactivateStream(sdr, txStream, 0, 0);
    SoapySDRDevice_closeStream(sdr, rxStream);
    SoapySDRDevice_closeStream(sdr, txStream);
    SoapySDRDevice_unmake(sdr);
    
    printf("FullMode завершена успешно\n");
}