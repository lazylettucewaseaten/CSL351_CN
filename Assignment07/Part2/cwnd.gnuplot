set terminal png size 1100,800
set xlabel 'time(s)'
set ylabel 'cwnd (Bytes)'
set grid

set output 'results/newreno_cwnd.png'
set title 'Newreno: congestion window vs time'
plot 'results/newreno_cwnd.txt' using 1:3 with lines linecolor rgb "blue" title 'Newreno cwnd'

set output 'results/highspeed_cwnd.png'
set title 'Highspeed: congestion window vs time'
plot 'results/highspeed_cwnd.txt' using 1:3 with lines linecolor rgb "red" title 'Highspeed cwnd'

set output 'results/veno_cwnd.png'
set title 'Veno: congestion window vs time'
plot 'results/veno_cwnd.txt' using 1:3 with lines linecolor rgb "dark-green" title 'Veno cwnd'

set output 'results/vegas_cwnd.png'
set title 'Vegas: congestion window vs time'
plot 'results/vegas_cwnd.txt' using 1:3 with lines linecolor rgb "purple" title 'Vegas cwnd'