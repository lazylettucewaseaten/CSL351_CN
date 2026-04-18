# Set global styling
set terminal pngcairo size 1000,600 enhanced font 'Verdana,11'
set grid
set key outside right top

# 1. Plot Congestion Window (cwnd) vs Time

set output 'as8_results/cwnd_plt.png'
set title 'Congestion Window: NewReno vs TcpAshish (8 Flows)'
set xlabel 'Time (Seconds)'
set ylabel 'Congestion Window (Bytes)'
set autoscale y

plot 'as8_results/Mixed_8_NewReno_cwnd.txt' using 1:3 with lines lw 2.5 lc rgb "#1b9e77" title 'NewReno', \
     'as8_results/Mixed_8_TcpAshish_cwnd.txt' using 1:3 with lines lw 2.5 lc rgb "#7570b3" title 'TcpAshish'

# 2. Plot Round Trip Time (RTT) vs Time

set output 'as8_results/rtt_plt.png'
set title 'Round Trip Time (RTT): NewReno vs TcpAshish (8 Flows)'
set xlabel 'Time (Seconds)'
set ylabel 'RTT (Seconds)'
set autoscale y

plot 'as8_results/Mixed_8_NewReno_rtt.txt' using 1:3 with lines lw 2.5 lc rgb "#1b9e77" title 'NewReno', \
     'as8_results/Mixed_8_TcpAshish_rtt.txt' using 1:3 with lines lw 2.5 lc rgb "#7570b3" title 'TcpAshish'

# 3. Plot Ssthresh vs Cwnd (To show Hystart trigger)

set output 'as8_results/ssthresh_plt.png'
set title 'TcpAshish: Hystart Trigger'
set xlabel 'Time (Seconds)'
set ylabel 'Bytes'

# Cap the Y-axis so the infinite starting ssthresh doesn't ruin the scale
set yrange [0:100000] 

plot 'as8_results/Mixed_8_TcpAshish_cwnd.txt' using 1:3 with lines lw 2.5 lc rgb "#7570b3" title 'TcpAshish Cwnd', \
     'as8_results/Mixed_8_TcpAshish_ssthresh.txt' using 1:3 with steps lw 3.0 lc rgb "#e7298a" title 'TcpAshish Ssthresh'


# 4. Plot Jain's Fairness Index (Bar Chart)

# hardocded the data fromt hhe terminal 
$FairnessData << EOD
"4 Flows"   0.9852    0.9999
"8 Flows"   0.9888    0.9887
"16 Flows"  0.9932    0.9915
"20 Flows"  0.9943    0.9966
EOD

set output 'as8_results/through_plt.png'
set title 'Jains Fairness Index Comparison'
set xlabel 'Total Number of Flows'
set ylabel 'Fairness Index'

# Set range to highlight the small differences in your data
set autoscale y
set yrange [0.95:1.01]

# Configure bar chart styling
set style data histograms
set style histogram cluster gap 1
set style fill solid 0.8 border -1
set boxwidth 0.9

plot $FairnessData using 2:xtic(1) lc rgb "#1b9e77" title 'NewReno (Baseline)', \
     $FairnessData using 3 lc rgb "#7570b3" title 'Mixed (NewReno + TcpAshish)'


print "plots in the as8_results folder."