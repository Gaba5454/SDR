import numpy as np
import matplotlib.pyplot as plt

# Чтение файлов
def read_pcm_file(filename):
    real = []
    imag = []
    with open(filename, "rb") as f:
        index = 0
        while (byte := f.read(2)):
            if index % 2 == 0:
                real.append(int.from_bytes(byte, byteorder='little', signed=True))
            else:
                imag.append(int.from_bytes(byte, byteorder='little', signed=True))
            index += 1
    return np.array(real), np.array(imag)

# Чтение данных
real_rx, imag_rx = read_pcm_file("rxdata.pcm")

# Создаем массивы индексов
count2 = np.arange(len(real_rx))

# Построение графиков TX и RX
plt.figure(figsize=(12, 8))


# RX сигнал
plt.subplot(3, 1, 2)
plt.plot(count2, real_rx, color='blue', label='I', linewidth=0.8)
plt.plot(count2, imag_rx, color='red', label='Q', linewidth=0.8)
plt.xlabel('Sample Index')
plt.ylabel('Amplitude')
plt.xlim(10000, 11500)
plt.legend()
plt.title('RX Signal I/Q Components')
plt.grid(True, alpha=0.3)


# BPSK созвездие (только точки с большой амплитудой в диапазоне 7910-9440)
real_rx_filtered = real_rx[10000:11500]
imag_rx_filtered = imag_rx[10000:11500]

start_offset = 6

real_decimated = real_rx_filtered[start_offset::10]
imag_decimated = imag_rx_filtered[start_offset::10]

plt.subplot(3, 1, 3)
plt.scatter(real_decimated, imag_decimated, alpha=0.7, s=20, color='green')
plt.xlabel('I component')
plt.ylabel('Q component')
plt.title('BPSK Constellation (Every 10th sample, 10000-11500)')
plt.grid(True, alpha=0.3)
plt.axis('equal')

plt.tight_layout()
plt.show()