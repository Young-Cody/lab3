import matplotlib.pyplot as plt
import numpy as np

plt.rcParams['font.family'] = 'STSong'

fig, ax = plt.subplots()
x = np.array([0, 0.02, 0.04, 0.06, 0.08, 0.1])
y1 = np.array([4210.6,
               129,
               134,
               128.16,
               127.55,
               130.43,

               ])
y2 = np.array([100.02,
               27.67,
               17.88,
               8.33,
               19.93,
               12.94

               ])
y3 = np.array([60.89,
               9.75,
               11.73,
               10.81,
               9.91,
               10.17

               ])
y4 = np.array([6675.41,
               383.45,
               299.61,
               175.4,
               80.39,
               93.1

               ])
y5 = np.array([113.86,
               87.64,
               87.59,
               83.69,
               74.73,
               76.56

               ])
y6 = np.array([88.77,
               59.87,
               55.6,
               51.65,
               49.57,
               48.25

               ])

ax.plot(x, y1, '^-b', label='停等机制 延迟时间=0')

ax.plot(x, y2, '<-g', label='停等机制 延迟时间=5ms')

ax.plot(x, y3, 'v-r', label='停等机制 延迟时间=10ms')

ax.plot(x, y4, 'v-b', label='滑动窗口机制 延迟时间=0')

ax.plot(x, y5, '^-g', label='滑动窗口机制 延迟时间=5ms')

ax.plot(x, y6, '<-r', label='滑动窗口机制 延迟时间=10ms')

ax.set_xlabel('丢包率(%)')
ax.set_ylabel('吞吐率(kbps)')
ax.set_title('停等机制和滑动窗口机制吞吐率对比')
ax.set_xticks([0, 0.02, 0.04, 0.06, 0.08, 0.1])
ax.set_yscale('log')
ax.legend()
fig.savefig('8.pdf')
