// functions.h

#include <stdio.h>
#include <math.h>
#include "struct.h"


/* Функция генерации случайной полседовательности 1 и 0.
Параметры:
array - массив в который залезут эти нолики и единички.
size - размер этого массива, сколько единичек и ноликов 
поместятся в этот массив. */

void generateRandomBits(int16_t *array, int size) {
    srand(time(NULL));
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

int16_t *pulseShaping(int16_t *array, int size, int sample) {
    int new_size = size * sample;
    int16_t *result = malloc(new_size * sizeof(int16_t));
    
    for(int i = 0; i < size; i++) {
        for(int j = 0; j < sample; j++) {
            result[i * sample + j] = array[i] * pulse_arr[j];
        }
    }
    return result;
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
    int16_t *result = malloc(2 * (size+8) * sizeof(int16_t));
    for(int i = 0; i < 4; i++){
        result[2*i] = (barker[i]*1500) << 4;
        result[2*i+1] = (barker[i]*1500 * 0) << 4;
    }
    for(int i = 4; i < size; i++) {
        result[2*i] = (arrayI[i] * 1500) << 4; 
        result[2*i+1] = (arrayQ[i] * 1500) << 4;
    }
    return result;
}

/* Согласованный фильтр*/
int16_t* matchedFilter(int16_t *signal, size_t size, int16_t *pulse_arr, int sample) {
    int16_t *result = (int16_t)calloc(size, sizeof(int16_t));
    for(int i = 0; i < size; i++) {
        for(int j = 0; j < sample; j++) {
            result[i * sample + j] = signal[i] * pulse_arr[j];
        }
    }
}
/* Я подаю на вход массив rx_buffers с комплексными отсчётами,
дальше мне нужно каждый раз перемножить по 10 элементов на 1
каждый. После чего получить массив I и Q но уже свернутые.









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