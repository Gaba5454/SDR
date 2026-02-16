#include <SoapySDR/Device.h>   // Инициализация устройства
#include <SoapySDR/Formats.h>  // Типы данных, используемых для записи сэмплов
#include <stdio.h>             // printf
#include <stdlib.h>            // free
#include <stdint.h>
#include <complex.h>
#include <time.h>
#include <math.h>

#define SIZE  192 
#define SAMPLE 10


#include <stdio.h>
#include <stdlib.h>

// Функция вычисляет линейную свёртку двух массивов a и b длины len_a и len_b.
// Возвращает указатель на массив-результат (его длина len_a + len_b - 1).
// Память под результат выделяется динамически, вызывающий должен её освободить.
int* convolution(int16_t *a, int16_t *b, int size) {
    int len_a = sizeof(a)/sizeof(a[0]);
    int len_b = sizeof(b)/sizeof(b[0]);
    int len_c = size + ((len_a/len_b) * 2)
    int *c = (int*)calloc(len_c, sizeof(int16_t)); // зануляем выделенную память
    if (c == NULL) {
        fprintf(stderr, "Ошибка выделения памяти\n");
        return NULL;
    }

    for (int i = 0; i < len_a; i++) {
        for (int j = 0; j < len_b; j++) {
            c[i + j] += a[i] * b[j];
        }
    }
    return c;
}

int main() {
    const int N = 10;
    int a[N], b[N];

    // Заполняем массивы единицами
    for (int i = 0; i < N; i++) {
        a[i] = 2;
        b[i] = 2;
    }

    int *result = convolution(a, N, b, N);
    if (result == NULL) {
        return 1; // ошибка уже выведена
    }

    int result_len = 2 * N - 1;
    printf("Результат свёртки:\n");
    for (int i = 0; i < result_len; i++) {
        printf("%d ", result[i]);
    }

    free(result);
    return 0;
}


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