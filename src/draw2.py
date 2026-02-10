import numpy as np
import matplotlib.pyplot as plt
from scipy.signal import convolve

# Открываем файл для чтения
name = "rxdata.pcm"
imag = []
real = []
count = []
counter = 0
with open(name, "rb") as f:
    index = 0
    while (byte := f.read(2)):
        if(index %2 == 0):
            real.append(int.from_bytes(byte, byteorder='little', signed=True))
            counter += 1
            count.append(counter)
        else:
            imag.append(int.from_bytes(byte, byteorder='little', signed=True))
        index += 1
# Преобразуем в numpy массивы
real = np.array(real)
imag = np.array(imag)
data = [1,1,1,1,1,1,1,1,1,1]

# Выполняем свертку для реальной и мнимой частей
real_conv = convolve(real, data, mode='same')
imag_conv = convolve(imag, data, mode='same')

# Инициализируем список для хранения данных


# fig, axs = plt.subplots(2, 1, layout='constrained')
plt.figure(1)
# axs[1].plot(count, np.abs(data),  color='grey')
plt.plot(count,(real_conv), color='blue')  
plt.figure(2)
# real_rx_filtered = real[10000:11500]
# imag_rx_filtered = imag[10000:11500]

# start_offset = 6
# real_decimated = real_rx_filtered[start_offset::10]
# imag_decimated = imag_rx_filtered[start_offset::10]
plt.scatter(real, imag, alpha=0.7, s=20, color='blue')
plt.show()