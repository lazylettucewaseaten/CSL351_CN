import pandas as pd
import matplotlib.pyplot as plt
import sys

if len(sys.argv) < 2:
    print("Usage: python3 q2plot.py <results.csv>")
    sys.exit(1)

df = pd.read_csv(sys.argv[1])

plt.figure(figsize=(10, 5))
color1 = 'tab:blue'
plt.plot(df['Second'], df['Throughput(Mbps)'], color=color1, marker='o', linestyle='-')
plt.xlabel('Time (Seconds)')
plt.ylabel('Throughput (Mbps)', color=color1)
plt.title('Network Performance: Throughput vs. Time')
plt.grid(True, linestyle='--', alpha=0.6)
plt.tight_layout()

plt.savefig('throughputplot.png', dpi=300)
print("Plot saved as 'throughputplot.png'")
plt.close() 

plt.figure(figsize=(10, 5))
color2 = 'tab:red'
plt.plot(df['Second'], df['AvgRTT(ms)'], color=color2, marker='x', linestyle='--')
plt.xlabel('Time (Seconds)')
plt.ylabel('Average RTT (ms)', color=color2)
plt.title('Network Performance: Average Delay vs. Time')
plt.grid(True, linestyle='--', alpha=0.6)
plt.tight_layout()

plt.savefig('delayplot.png', dpi=300)
print("Plot saved as 'delayplot.png'")
plt.close()