// functions.h

#include <stdio.h>
#include <math.h>
#include "struct.h"
#include <stdlib.h>
 
/* Функция генерации случайной полседовательности 1 и 0.
Параметры:
array - массив в который залезут эти нолики и единички.
size - размер этого массива, сколько единичек и ноликов 
поместятся в этот массив. */
void generateRandomBits(int16_t *array, int size) {
    for(int i = 0; i < size; i++) {
        array[i] = rand() % 2;
    }
}

/* Функция формирующего фильтра.
Фильтр увеличивает массив в SAMPLE раз и на выход выдает
SIZE * SAMPLE чисел свернутых с импульсной хар-ой.
Параметры:
array - входной массив который надо свернуть. 
size - размер исходного массива.
sample - во сколько раз нужно увеличить массив.
Выход:
result - массив после апсемплинга и свертки.*/
int16_t *upSampling(int16_t *array, int size, int sample) {
    int new_size = size * sample;
    int16_t *result = calloc(new_size, sizeof(int16_t));
    
    for(int i = 0; i < size; i++){
        for(int j = 0; j < sample; j++){
            result[i*SAMPLE] = array[i];
        }
    }
    return result;
}

void convolvePulse(int16_t *a, int len_a, int16_t *b, int len_b, int16_t* output) {
    int result_len = len_a + len_b - 1;
    
    for (int i = 0; i < result_len; i++) {
        int16_t sum = 0; 
        for (int j = 0; j < len_b; j++) {
            int idx = i - j;
            if (idx >= 0 && idx < len_a) {
                sum += (int16_t)a[idx] * (int16_t)b[j];
            }
        }
        output[i] = sum;

    }
}

// Мне очень нравится эта функция, она идеальна.

/* Кадровая синхронизация использующая последовательность Баркера
Обращаю внимание на то, что мы умножаем каждый элемент на 1500 и сдвигаем 
на 4 бита влево - это нужно для работы Adalm pluto(нужно уточнить).
Объединяем два массива в один.
Параметры:
arrayI - массив значений квадратуры I.
arrayQ - массив значений квадратуры Q.
size - размер этих двух массивов.
Выход:
result - массив значений на TX.*/

int16_t *acp(int16_t *arrayI, int16_t *arrayQ, int size) {
    int16_t *result = malloc(2 * size * sizeof(int16_t));

    for(int i = 0; i < size; i++) {
        result[2*i] = (arrayI[i] * 1500) << 4; 
        result[2*i+1] = (arrayQ[i] * 1500*0) << 4;
    }
    return result;
}


/* Согласованный фильтр*/
void convolveMatched(int16_t *a, int len_a, int16_t *b, int len_b, int32_t* output) {
    int result_len = len_a + len_b - 1;
    
    for (int i = 0; i < result_len; i++) {
        int32_t sum = 0; 
        for (int j = 0; j < len_b; j++) {
            int idx = i - j;
            if (idx >= 0 && idx < len_a) {
                sum += (int32_t)a[idx] * (int32_t)b[j];
            }
        }
        output[i] = sum;
    }
}

/* 
Демаппер BPSK
Простое решение по знаку вещественной части (для BPSK этого достаточно)
*/
void demap_bpsk(int32_t *sig_i, int32_t *sig_q, int *indices, int count, int16_t *out_bits) {
    for (int i = 0; i < count; i++) {
        int idx = indices[i];
        // Если I > 0 -> бит 1, иначе 0 (зависит от твоей маппинг-таблицы)
        out_bits[i] = (sig_i[idx] > 0) ? 1 : 0;
    }
}

/* Функция из лабы с передачей звукового файла*/
int16_t *read_pcm(const char *filename, size_t *sample_count) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Error: Cannot open file %s\n", filename);
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    printf("file_size = %ld\\n", file_size);
    int16_t *samples = (int16_t *)malloc(file_size);

    *sample_count = file_size / sizeof(int16_t);

    size_t sf = fread(samples, sizeof(int16_t), *sample_count, file);

    fclose(file);

    return samples;
}