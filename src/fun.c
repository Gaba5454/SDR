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
    for(int iter = 0; iter < 1; iter++){
        int16_t bits[SIZE];
        generateRandomBits(bits,SIZE);


        IQComponent Mapp;
        BPSK(bits,SIZE, &Mapp);

        int16_t *pulseI = pulseShaping(Mapp.Im, SIZE, SAMPLE);
        int16_t *pulseQ = pulseShaping(Mapp.Qa, SIZE, SAMPLE);

        int16_t *arrayForTX = acp(pulseI, pulseQ, SIZE * SAMPLE);
        for(int i = 0; i < SIZE * SAMPLE * 2; i++) { 
            fwrite(&arrayForTX[i], sizeof(int16_t), 1, file1);  
        }
  
int16_t *deinterleaved_i = malloc(SIZE * SAMPLE * sizeof(int16_t));
int16_t *deinterleaved_q = malloc(SIZE * SAMPLE * sizeof(int16_t));

for (int idx = 0; idx < SIZE * SAMPLE; idx++) {
    deinterleaved_i[idx] = arrayForTX[2 * idx];     // I: 0, 2, 4...
    deinterleaved_q[idx] = arrayForTX[2 * idx + 1]; // Q: 1, 3, 5...
}


int32_t *out_i = calloc(matched_len, sizeof(int32_t));
int32_t *out_q = calloc(matched_len, sizeof(int32_t));

matchedFilter(deinterleaved_i, SIZE * SAMPLE, pulse_arr, SAMPLE, out_i);
matchedFilter(deinterleaved_q, SIZE * SAMPLE, pulse_arr, SAMPLE, out_q);
        for(int i = 0; i < matched_len; i++){
                int16_t out_i_16 = (int16_t)(out_i[i] / 1000);  // Масштабирование
                int16_t out_q_16 = (int16_t)(out_q[i] / 1000);
                fwrite(&out_i_16, sizeof(int16_t), 1, file);
                fwrite(&out_q_16, sizeof(int16_t), 1, file);
        }
    
        free(pulseI);
        free(pulseQ);
        free(arrayForTX);
        free(deinterleaved_i);
        free(deinterleaved_q);
    }
    fclose(file);
    fclose(file1);  
    return 0;
}
/*
// Функция вычисляет линейную свёртку двух массивов a и b длины len_a и len_b.
// Возвращает указатель на массив-результат (его длина len_a + len_b - 1).
// Память под результат выделяется динамически, вызывающий должен её освободить.
int* convolution(int16_t *a, int len_a, int16_t *b, int len_b, int size) {
    int *c = (int*)calloc(size, sizeof(int16_t)); // зануляем выделенную память
    if (c == NULL) {
        fprintf(stderr, "Ошибка выделения памяти\n");
        return NULL;
    }

    for (int i = 0; i < len_a; i++) {
        for (int j = 0; j < len_b; j++) {
            c[i] += a[i+j] * b[j];
        }
    }
    return c;
}

*/
/* Фильтр matched от ИИ
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
*/

/* Функция создания комплексного сигнала
void create_complex_signal(int *arrayI, int *arrayQ, double *t, double *signalI, double *signalQ) {
    int *bigI = pulseShaping(arrayI, SIZE);
    int *bigQ = pulseShaping(arrayQ, SIZE);
    
    for(int i = 0; i < Ns; i++) {
        signalI[i] = bigI[i] * cos(2 * pi * fsym * t[i]);
        signalQ[i] = bigQ[i] * sin(2 * pi * fsym * t[i]);
    }
    
    free(bigI);
    free(bigQ);
}*/