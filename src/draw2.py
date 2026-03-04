import numpy as np
import matplotlib.pyplot as plt
import random

# Открываем файл для чтения
name = "../build/rxdata.pcm"

data = []
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
            imag.append(int.from_bytes(byte*0, byteorder='little', signed=True)) # Для BPSK byte умножаем на 0
        index += 1
        
# Инициализируем список для хранения данных
idx=0
real = real[idx:]
imag = imag[idx:]
k=np.ones(10)
#itog = np.convolve(real,k,'same')
# fig, axs = plt.subplots(2, 1, layout='constrained')
plt.figure(1)
# axs[1].plot(count, np.abs(data),  color='grey')  # Используем scatter для диаграммы созвездия
#plt.plot((imag),color='red')  # Используем scatter для диаграммы созвездия
plt.plot(imag, color = 'red')
plt.plot(real, color='blue')  # Используем scatter для диаграммы созвездия
plt.figure(2)
plt.scatter(real,imag)
plt.show()