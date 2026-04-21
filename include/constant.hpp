// constant.hpp

#pragma once

#include <array>
#include <cstdint>
#include <cmath>

// constexpr вместо #define
constexpr int SIZE = 192;
constexpr int SAMPLE = 10;
constexpr double PI = 3.141592653589793;
constexpr int BARKER_LEN = 13;

// Глобальные константные массивы
inline constexpr std::array<int16_t, SAMPLE> PULSE_ARR = {1,1,1,1,1,1,1,1,1,1};
inline constexpr std::array<int16_t, BARKER_LEN> BARKER_13 = {1, 1, 1, 1, 1, -1, -1, 1, 1, -1, 1, -1, 1};

// Структура параметров сигнала
struct SignalParameters {
    double R{3.0};
    double T{1.0};
    double fsym{1.0 / T};
    double fd{SAMPLE * fsym};
    double Ts{1.0 / fd};
    int Ns{SIZE * SAMPLE};
    
    // Пересчёт при изменении параметров
    void recalculate() {
        T = 1.0 / R;
        fsym = 1.0 / T;
        fd = SAMPLE * fsym;
        Ts = 1.0 / fd;
        Ns = SIZE * SAMPLE;
    }
};

// Компоненты квадратур
struct IQComponent {
    std::array<int16_t, SIZE> Im{};
    std::array<int16_t, SIZE> Qa{};
    
    void clear() {
        Im.fill(0);
        Qa.fill(0);
    }
};

// Комплексное число для внутренней обработки
struct ComplexSample {
    int32_t i{};
    int32_t q{};
    
    ComplexSample operator*(int32_t scalar) const {
        return {i * scalar, q * scalar};
    }
    
    int64_t energy() const {
        return static_cast<int64_t>(i) * i + static_cast<int64_t>(q) * q;
    }
};