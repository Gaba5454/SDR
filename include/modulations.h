// modulations.h

#include <stdio.h>
#include <math.h>
#include "struct.h"

/* BPSK mapper-функция реализует преобразования битов в символы
квадратур I и Q.
Параметры:
array - массив из которого берутся единички и нолики.
size - размер выходного массива(не может отличаться от
размера входного).
Выход:
result - структура с двумя значениями Im и Qa. */

IQComponent BPSK(int *array, int size) {
    IQComponent result;
    for(int i = 0; i < size; i++) {
        if(array[i] == 0) {
            result.Im[i] = -1;
            result.Qa[i] = 0;
        }
        else {
            result.Im[i] = 1;
            result.Qa[i] = 0;
        }
    }
    return result;
}
