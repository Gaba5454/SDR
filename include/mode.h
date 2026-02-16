// mode.h

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
    SoapySDRDevice_setGain(sdr, SOAPY_SDR_RX, channels[0], 40.0); // Чувствительность приемника
    SoapySDRDevice_setGain(sdr, SOAPY_SDR_TX, channels[0], -10.0);// Усиление передатчика
    
    size_t channel_count = sizeof(channels) / sizeof(channels[0]);
    // Формирование потоков для передачи и приема сэмплов
    SoapySDRStream *rxStream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_RX, SOAPY_SDR_CS16, channels, channel_count, NULL);
    SoapySDRStream *txStream = SoapySDRDevice_setupStream(sdr, SOAPY_SDR_TX, SOAPY_SDR_CS16, channels, channel_count, NULL);
    
    SoapySDRDevice_activateStream(sdr, rxStream, 0, 0, 0); //start streaming
    SoapySDRDevice_activateStream(sdr, txStream, 0, 0, 0); //start streaming
    //
    // Получение MTU (Maximum Transmission Unit), в нашем случае - размер буферов. 
    size_t rx_mtu = SoapySDRDevice_getStreamMTU(sdr, rxStream);
    size_t tx_mtu = SoapySDRDevice_getStreamMTU(sdr, txStream);
    printf("\n\ntx_mtu = %zu\n", tx_mtu);
    // int16_t tx_buff[2*tx_mtu]; // Original
    int16_t rx_buffer[2*rx_mtu];
    size_t count = 0;
    int16_t *tx_buff = array;
    FILE *file1 = fopen("txdata.pcm", "wb");
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

    // Начинается работа с получением и отправкой сэмплов
    FILE *file = fopen("rxdata.pcm", "wb");

    for (size_t buffers_read = 0; buffers_read < iteration_count; buffers_read++) {
        
        void *rx_buffs[] = {rx_buffer};
        int flags;        // flags set by receive operation
        long long timeNs; //timestamp for receive buffer
        
        // считали буффер RX, записали его в rx_buffer
        int sr = SoapySDRDevice_readStream(sdr, rxStream, rx_buffs, rx_mtu, &flags, &timeNs, timeoutUs);
        // ЗАПИСЬ ДАННЫХ В ФАЙЛ - ВСТАВЛЕННЫЙ КОД
        for(int i = 0; i < rx_mtu * 2; i++){
        if(rx_buffer[i] > 500 || rx_buffer[i] < -500){
            fwrite(&rx_buffer[i], sizeof(int16_t), 1, file);
            }
        }
        
        // Смотрим на количество считаных сэмплов, времени прихода и разницы во времени с чтением прошлого буфера
        printf("Buffer: %lu - Samples: %i, Flags: %i, Time: %lli, TimeDiff: %lli\n", buffers_read, sr, flags, timeNs, timeNs - last_time);
        last_time = timeNs;

        // Переменная для времени отправки сэмплов относительно текущего приема
        long long tx_time = timeNs + (6 * 1000 * 1000); // на 4 [мс] в будущее
        // Добавляем время, когда нужно передать блок tx_buff, через tx_time -наносекунд
        
        // Здесь отправляем наш tx_buff массив
        void *tx_buffs[] = {tx_buff};
        printf("\n");
        printf("%d\n", 1920 * buffers_read * 2);
        
        for(size_t i = 0; i < 8; i++) {
            uint8_t tx_time_byte = (tx_time >> (i * 8)) & 0xff;
            tx_buff[2 + i] = tx_time_byte << 4;
        }
            printf("buffers_read: %d\n", buffers_read);
            flags = SOAPY_SDR_HAS_TIME;
            int st = SoapySDRDevice_writeStream(sdr, txStream, (const void * const*)tx_buffs, tx_mtu, &flags, tx_time, timeoutUs);
    }
    if (file) fclose(file);
    //stop streaming
    SoapySDRDevice_deactivateStream(sdr, rxStream, 0, 0);
    SoapySDRDevice_deactivateStream(sdr, txStream, 0, 0);

    //shutdown the stream
    SoapySDRDevice_closeStream(sdr, rxStream);
    SoapySDRDevice_closeStream(sdr, txStream);

    //cleanup device handle
    SoapySDRDevice_unmake(sdr);
}