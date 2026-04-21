// struct.h

#include <stdio.h>
#include <stdint.h>

#define SIZE 192
#define SAMPLE 10
#define PI 3.14
#define BARKER_LEN 13
int16_t const pulse_arr[SAMPLE] = {1,1,1,1,1,1,1,1,1,1};
int16_t BARKER_13[BARKER_LEN] = {1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1};
 
// Структура с инициализацией массивов I и Q квадратур. 
typedef struct {
    int16_t Im[SIZE+BARKER_LEN];
    int16_t Qa[SIZE+BARKER_LEN];
} IQComponent; 

