// functions.h

#include <stdio.h>
#include <math.h>
#include "modulations.h"


/* Функция генерации случайной полседовательности 1 и 0.
Параметры:
array - массив в который залезут эти нолики и единички.
size - размер этого массива, сколько единичек и ноликов 
поместятся в этот массив. */

void generateRandomBits(int *array, int size) {
    for(int i = 0; i < size; i++) {
        array[i] = rand() % 2;
    }
}

// Насколько сильно нужен кодер тут?

/* Функция формирующего фильтра.
Фильтр увеличивает массив в SAMPLE раз и на выход выдает
SIZE * SAMPLE чисел свернутых с импульсной хар-ой.
Параметры:
array - входной массив который надо свернуть. 
size - размер исходного массива.
sample - во сколько раз нужно увеличить массив.
Выход:
result - массив после апсемплинга и свертки.*/

int *pulseShaping(int *array, int size, int sample) {
    int new_size = size * sample;
    int *result = malloc(new_size * sizeof(int));
    
    for(int i = 0; i < size; i++) {
        for(int j = 0; j < sample; j++) {
            result[i * sample + j] = array[i] * pulse_arr[j];
        }
    }
    return result;
}
// Мне очень нравится эта функция, она идеальна.

