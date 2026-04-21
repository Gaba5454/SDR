// functions.h

#include <stdio.h>
#include <math.h>
#include "struct.h"
#include <stdint.h>
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
        result[2*i+1] = (arrayQ[i] * 1500) << 4;
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


// Написать код для TED который будет вычислять по формуле оптимальную точку взятия числа.
// После написать функцию которая выбирает один из 10 отсчётов каждого SAMPLE-а и записывает данные в новый массив

// Написать функцию которая делает частотную синхронизацию
// functions.h

// Gardner TED - вычисление ошибки для конкретного offset
int64_t compute_gardner_error(int16_t *I, int16_t *Q, int len, int Nsps, int offset) {
    int64_t error_sum = 0;
    int count = 0;
    
    // Проходим по всем символам с данным offset
    for (int n = offset; n + Nsps + Nsps/2 < len; n += Nsps) {
        // Формула из документа:
        // e = (I(n+Nsp+Ns) - I(n+Ns)) * I(n+Nsp/2+Ns) + 
        //     (Q(n+Nsp+Ns) - Q(n+Ns)) * Q(n+Nsp/2+Ns)
        
        int idx_early = n;                    // I(n+Ns)
        int idx_mid   = n + Nsps/2;           // I(n+Nsp/2+Ns)
        int idx_late  = n + Nsps;             // I(n+Nsp+Ns)
        
        int32_t diff_I = (int32_t)I[idx_late] - (int32_t)I[idx_early];
        int32_t diff_Q = (int32_t)Q[idx_late] - (int32_t)Q[idx_early];
        
        int32_t e = diff_I * (int32_t)I[idx_mid] + diff_Q * (int32_t)Q[idx_mid];
        
        error_sum += (int64_t)e * e;  // квадрат ошибки
        count++;
    }
    
    return (count > 0) ? error_sum : INT64_MAX;
}

// Поиск лучшего offset
int find_best_offset(int16_t *I, int16_t *Q, int len, int Nsps) {
    int best_offset = 0;
    int64_t min_error = INT64_MAX;
    
    // Пробуем все offset от 0 до Nsps-1
    for (int offset = 0; offset < Nsps; offset++) {
        int64_t err = compute_gardner_error(I, Q, len, Nsps, offset);
        
        if (err < min_error) {
            min_error = err;
            best_offset = offset;
        }
    }
    
    return best_offset;
}


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/*
 * Costas Loop для частотной синхронизации (BPSK/QPSK)
 * 
 * Принцип работы:
 * 1. NCO генерирует sin/cos по текущей фазе
 * 2. Входной сигнал смешивается с несущей (ротация)
 * 3. Детектор фазовой ошибки: e = I * Q (для BPSK)
 * 4. PI-фильтр обновляет фазу: phase += Kp*e + Ki*integral
 * 
 * Параметры (подбираются под ваш сигнал):
 * - loop_bw: ширина петли (0.01...0.1), чем меньше — тем медленнее, но стабильнее
 * - damping: коэффициент демпфирования (~1.0 для критического затухания)
 */

void costas_loop_sync(const char *input_file, const char *output_file, double loop_bw, double damping) {
    
    FILE *fin = fopen(input_file, "rb");
    
    // === Проверка размера файла ===
    fseek(fin, 0, SEEK_END);
    long file_size = ftell(fin);
    fseek(fin, 0, SEEK_SET);
    
    printf("Input file size: %ld bytes\n", file_size);
    printf("Expected complex samples: %ld\n", file_size / 4);  // 4 байта на I+Q
    
    FILE *fout = fopen(output_file, "wb");

    // === Параметры петли ===
    double Kp = damping * loop_bw;
    double Ki = loop_bw * loop_bw / 4.0;
    
    double phase = M_PI;
    double phase_integrator = 0.0;
    
    // === Читаем ВСЕ данные сразу для надёжности ===
    int16_t *buffer = malloc(file_size);
    
    size_t samples_read = fread(buffer, sizeof(int16_t), file_size / sizeof(int16_t), fin);
    printf("Read %zu int16_t values (%zu complex samples)\n", 
           samples_read, samples_read / 2);
    
    // === Обрабатываем попарно ===
    size_t complex_samples = 0;
    for (size_t i = 0; i + 1 < samples_read; i += 2) {
        
        double I_in = (double)buffer[i] / 32768.0;
        double Q_in = (double)buffer[i+1] / 32768.0;
        
        double nco_cos = cos(phase);
        double nco_sin = sin(phase);
        
        double I_mix = I_in * nco_cos + Q_in * nco_sin;
        double Q_mix = Q_in * nco_cos - I_in * nco_sin;
        
        double phase_error = I_mix * Q_mix;
        
        phase_integrator += Ki * phase_error;
        double freq_correction = Kp * phase_error + phase_integrator;
        
        phase += freq_correction;
        while (phase > M_PI) phase -= 2.0 * M_PI;
        while (phase < -M_PI) phase += 2.0 * M_PI;
        
        int16_t I_out = (int16_t)(I_mix * 32768.0 * 0.9);
        int16_t Q_out = (int16_t)(Q_mix * 32768.0 * 0.9);
        
        fwrite(&I_out, sizeof(int16_t), 1, fout);
        fwrite(&Q_out, sizeof(int16_t), 1, fout);
        
        complex_samples++;
    }
    
    printf("Processed %zu complex samples\n", complex_samples);
    printf("Output file size: %ld bytes\n", complex_samples * 4);
    free(buffer);
    fclose(fin);
    fclose(fout);
}
/*
 * Функция коррекции полярности (инверсии) сигнала на основе слова Баркера.
 * Если корреляция с Баркером отрицательная и большая по модулю, значит сигнал инвертирован.
 * Умножаем весь сигнал на -1.
 */
void correct_signal_polarity(const char *input_file, const char *output_file) {
    FILE *fin = fopen(input_file, "rb");
    if (!fin) {
        fprintf(stderr, "Error: Cannot open %s\n", input_file);
        return;
    }

    fseek(fin, 0, SEEK_END);
    long file_size = ftell(fin);
    fseek(fin, 0, SEEK_SET);

    int16_t *iq_data = malloc(file_size);
    if (!iq_data) { fclose(fin); return; }

    size_t items_read = fread(iq_data, sizeof(int16_t), file_size / sizeof(int16_t), fin);
    fclose(fin);

    int total_complex = items_read / 2;
    if (total_complex < BARKER_LEN) { free(iq_data); return; }

    // Ищем корреляцию только по I-каналу (так как BPSK)
    double max_corr = -1e9;
    double min_corr = 1e9;
    
    for (int pos = 0; pos <= total_complex - BARKER_LEN; pos++) {
        double corr = 0.0;
        for (int chip = 0; chip < BARKER_LEN; chip++) {
            // iq_data[pos*2] - это I компонента
            corr += (double)iq_data[(pos + chip) * 2] * (double)BARKER_13[chip];
        }
        if (corr > max_corr) max_corr = corr;
        if (corr < min_corr) min_corr = corr;
    }

    printf("Polarity Check: MaxCorr=%.0f, MinCorr=%.0f\n", max_corr, min_corr);

    // Если минимальная корреляция (отрицательная) больше по модулю, чем максимальная положительная,
    // значит сигнал пришел в противофазе (инвертирован).
    int needs_inversion = 0;
    if (fabs(min_corr) > max_corr) {
        needs_inversion = 1;
        printf("Signal is INVERTED. Correcting polarity...\n");
    } else {
        printf("Signal polarity is OK.\n");
    }

    FILE *fout = fopen(output_file, "wb");
    if (!fout) { free(iq_data); return; }

    for (size_t i = 0; i < items_read; i++) {
        int16_t val = iq_data[i];
        if (needs_inversion) {
            val = -val;
        }
        fwrite(&val, sizeof(int16_t), 1, fout);
    }

    fclose(fout);
    free(iq_data);
}

void extract_after_barker(const char *input_file, const char *output_file, int num_symbols) {
    
    FILE *fin = fopen(input_file, "rb");
    if (!fin) {
        fprintf(stderr, "Error: Cannot open input file %s\n", input_file);
        return;
    }
    
    fseek(fin, 0, SEEK_END);
    long file_size = ftell(fin);
    fseek(fin, 0, SEEK_SET);
    
    int16_t *iq_data = malloc(file_size);
    if (!iq_data) { fclose(fin); return; }
    
    size_t items_read = fread(iq_data, sizeof(int16_t), file_size / sizeof(int16_t), fin);
    fclose(fin);
    
    int total_complex = items_read / 2;
    
    // 1. Поиск Баркера и определение полярности
    double max_corr = -1e9;
    double min_corr = 1e9;
    int best_pos = -1;
    
    for (int pos = 0; pos <= total_complex - BARKER_LEN; pos++) {
        double corr = 0.0;
        for (int chip = 0; chip < BARKER_LEN; chip++) {
            // Коррелируем только по I-каналу (BPSK)
            corr += (double)iq_data[(pos + chip) * 2] * (double)BARKER_13[chip];
        }
        
        if (corr > max_corr) {
            max_corr = corr;
            best_pos = pos;
        }
        if (corr < min_corr) {
            min_corr = corr;
        }
    }
    
    printf("Barker found at: %d\n", best_pos);
    printf("Max Corr: %.2f, Min Corr: %.2f\n", max_corr, min_corr);

    // Определяем, нужно ли инвертировать сигнал
    int invert = 0;
    if (fabs(min_corr) > max_corr) {
        invert = 1;
        printf(">>> INVERSION DETECTED! Correcting signal polarity...\n");
    } else {
        printf(">>> Polarity is OK.\n");
    }
    
    int data_start = best_pos + BARKER_LEN;
    
    if (data_start + num_symbols > total_complex) {
        fprintf(stderr, "Warning: not enough data after Barker\n");
        num_symbols = total_complex - data_start;
        if (num_symbols <= 0) { free(iq_data); return; }
    }
    
    // 2. Запись с учетом возможной инверсии
    FILE *fout = fopen(output_file, "wb");
    if (!fout) { free(iq_data); return; }
    
    int start_idx = data_start * 2; // Начало данных в массиве int16_t
    int count = num_symbols * 2;    // Количество элементов int16_t (I+Q)
    
    for (int i = 0; i < count; i++) {
        int16_t val = iq_data[start_idx + i];
        if (invert) {
            val = -val; // Инвертируем биты (фазу) если нужно
        }
        fwrite(&val, sizeof(int16_t), 1, fout);
    }
    
    fclose(fout);
    free(iq_data);
    
    printf("Extracted %d symbols to %s (Inverted: %s)\n", 
           num_symbols, output_file, invert ? "YES" : "NO");
}

void demapper_and_compare(const char *input_file, int16_t *tx_bits, int num_symbols) {
    
    FILE *fin = fopen(input_file, "rb");
    if (!fin) {
        fprintf(stderr, "Error: cannot open %s\n", input_file);
        return;
    }
    
    // Читаем данные
    int16_t *rx_iq = malloc(num_symbols * 2 * sizeof(int16_t));
    size_t read_count = fread(rx_iq, sizeof(int16_t), num_symbols * 2, fin);
    fclose(fin);
    
    if (read_count != num_symbols * 2) {
        fprintf(stderr, "Warning: expected %d values, read %zu\n", num_symbols * 2, read_count);
    }
    
    // === Демаппинг: BPSK (I > 0 → 1, I < 0 → 0) ===
    int16_t *rx_bits = malloc(num_symbols * sizeof(int16_t));
    
    for (int i = 0; i < num_symbols; i++) {
        if (rx_iq[i * 2] > 0) {
            rx_bits[i] = 1;
        } else {
            rx_bits[i] = 0;
        }
    }
    
    // === Сравнение с переданными битами ===
    int errors = 0;
    int max_errors_to_store = num_symbols;  // Выделяем достаточно места
    int *error_positions = malloc(max_errors_to_store * sizeof(int));  // Динамически
    int error_count = 0;
    
    for (int i = 0; i < num_symbols; i++) {
        // Приводим TX к 0/1: +1 → 1, -1 → 0
        int tx_bit = (tx_bits[i] > 0) ? 1 : 0;  // ← БЫЛО: (tx_bits[i] < 0) ? 1 : 0
        int rx_bit = rx_bits[i];
        
        if (rx_bit != tx_bit) {
            errors++;
            if (error_count < max_errors_to_store) {
                error_positions[error_count++] = i;
            }
        }
    }
    
    double ber = (double)errors / num_symbols;
    
    // === Вывод статистики ===
    printf("\n");
    printf("========================================\n");
    printf("       DEMAPPER & COMPARISON RESULT    \n");
    printf("========================================\n");
    printf("Total symbols:     %d\n", num_symbols);
    printf("Bit errors:        %d\n", errors);
    printf("BER:               %.6f (%.2f%%)\n", ber, ber * 100.0);
    
    if (errors == 0) {
        printf("Status:            ✅ PERFECT! No errors!\n");
    } else if (ber < 0.01) {
        printf("Status:            ✅ GOOD! BER < 1%%\n");
    } else if (ber < 0.1) {
        printf("Status:            ⚠️  ACCEPTABLE. BER < 10%%\n");
    } else {
        printf("Status:            ❌ BAD! BER >= 10%%\n");
    }
    
    // === Вывод первых ошибок ===
    if (errors > 0 && errors <= 20) {
        printf("\nError positions:\n");
        for (int i = 0; i < error_count; i++) {
            int tx_bit = (tx_bits[error_positions[i]] > 0) ? 1 : 0;
            printf("  Symbol %3d: TX=%d, RX=%d (I=%d)\n", 
                   error_positions[i], 
                   tx_bit,
                   rx_bits[error_positions[i]],
                   rx_iq[error_positions[i] * 2]);
        }
    } else if (errors > 20) {
        printf("\nFirst 10 error positions:\n");
        for (int i = 0; i < 10; i++) {
            int tx_bit = (tx_bits[error_positions[i]] > 0) ? 1 : 0;
            printf("  Symbol %3d: TX=%d, RX=%d (I=%d)\n", 
                   error_positions[i], 
                   tx_bit,
                   rx_bits[error_positions[i]],
                   rx_iq[error_positions[i] * 2]);
        }
        printf("  ... and %d more errors\n", errors - 10);
    }
    
    // === Вывод первых 20 бит для сверки ===
    printf("\nFirst 192 bits comparison:\n");
    printf("TX: ");
    for (int i = 0; i < 192 && i < num_symbols; i++) {
        printf("%d", (tx_bits[i] > 0) ? 1 : 0);
    }
    printf("\nRX: ");
    for (int i = 0; i < 192 && i < num_symbols; i++) {
        printf("%d", rx_bits[i]);
    }
    printf("\n");
    
    printf("========================================\n\n");
    
    free(rx_iq);
    free(rx_bits);
    free(error_positions);  // Освобождаем память
}

// ================
void debug_signal(const char *filename, int num_samples) {
    FILE *f = fopen(filename, "rb");
    if (!f) return;
    
    int16_t *iq = malloc(num_samples * 4);
    fread(iq, sizeof(int16_t), num_samples * 2, f);
    fclose(f);
    
    printf("\n=== Signal Debug (%d samples) ===\n", num_samples);
    printf("Symbol |   I   |   Q   | Magnitude\n");
    printf("-------|-------|-------|----------\n");
    
    int min_I = 32767, max_I = -32768;
    int min_Q = 32767, max_Q = -32768;
    
    for (int i = 0; i < num_samples; i++) {
        int I = iq[i * 2];
        int Q = iq[i * 2 + 1];
        double mag = sqrt(I*I + Q*Q);
        
        if (I < min_I) min_I = I;
        if (I > max_I) max_I = I;
        if (Q < min_Q) min_Q = Q;
        if (Q > max_Q) max_Q = Q;
        
        if (i < 20) {  // Первые 20 символов
            printf("  %3d  | %5d | %5d | %7.1f\n", i, I, Q, mag);
        }
    }
    
    printf("\nI range: [%d, %d]\n", min_I, max_I);
    printf("Q range: [%d, %d]\n", min_Q, max_Q);
    printf("=================================\n\n");
    
    free(iq);
}