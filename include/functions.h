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
            if(i < 13){
                result[i * sample + j] = BARKER_13[i];
            }
            else {
                result[i * sample + j] = array[i] * pulse_arr[j];
            }
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
    int16_t *result = malloc(2 * size * sizeof(int16_t));

    for(int i = 0; i < size; i++) {
        result[2*i] = (arrayI[i] * 1500) << 4; 
        result[2*i+1] = (arrayQ[i] * 1500) << 4;
    }
    return result;
}
/*int16_t *acp(int16_t *arrayI, int16_t *arrayQ, int size) {
    

    
    // Общий размер: преамбула (13 символов) + полезные данные (size символов)
    int total_symbols = BARKER_LEN + size;
    
    // Выделяем память под интерливинговый массив: 2 компонента (I,Q) на символ
    int16_t *result = malloc(2 * total_symbols * sizeof(int16_t));
    if (result == NULL) {
        fprintf(stderr, "Ошибка: не удалось выделить память в acp()\n");
        return NULL;
    }
    
    // === 1. Записываем код Баркера в начало (символы 0..12) ===
    for (int i = 0; i < BARKER_LEN; i++) {
        // I-канал: значение Баркера × масштаб × сдвиг для Pluto
        result[2 * i] = (BARKER_13[i] * 1500) << 4;
        // Q-канал: для BPSK = 0 (но сохраняем формат для совместимости)
        result[2 * i + 1] = (BARKER_13[i] * 1500) << 4;  // Или (BARKER_13[i] * 0) << 4 если нужно
    }
    
    // === 2. Записываем полезные данные после преамбулы (символы 13..13+size-1) ===
    for (int i = 0; i < size; i++) {
        int sym_idx = BARKER_LEN + i;  // Индекс символа в result с учётом преамбулы
        
        result[2 * sym_idx]     = (arrayI[i] * 1500) << 4;  // I-компонента
        result[2 * sym_idx + 1] = (arrayQ[i] * 1500) << 4;  // Q-компонента
    }
    
    return result;
}*/


/* Согласованный фильтр*/
void matchedFilter(int16_t *a, int len_a, int16_t *b, int len_b, int32_t* output) {
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
 * Gardner Timing Error Detector с PLL-фильтром
 * Возвращает массив индексов оптимальных точек выборки
 * matched: входной массив после matched filter (комплексный)
 * matched_len: длина входного массива
 * samples_per_symbol: SAMPLE (количество отсчётов на символ)
 * out_len: выходной параметр - длина возвращаемого массива
 */
int findOptimalOffset(int32_t *signal_i, int32_t *signal_q, int signal_len, 
                      int samples_per_symbol, int symbols_count, int *out_indices) {
    
    int found = 0;
    for (int sym = 0; sym < symbols_count; sym++) {
        int start = sym * samples_per_symbol;
        int end = start + samples_per_symbol;
        if (end > signal_len) break;
        
        int best_idx = start;
        int32_t max_energy = 0;
        for (int idx = start; idx < end; idx++) {
            int32_t energy = signal_i[idx] * signal_i[idx] + signal_q[idx] * signal_q[idx];
            if (energy > max_energy) {
                max_energy = energy;
                best_idx = idx;
            }
        }
        out_indices[found++] = best_idx;
    }
    return found;
}

    /*int16_t *c = (int16_t*)calloc(size, sizeof(int16_t)); // зануляем выделенную память
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
}*/

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