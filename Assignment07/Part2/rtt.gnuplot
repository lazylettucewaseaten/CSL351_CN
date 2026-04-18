set terminal png size 1100,800
set xlabel 'time (s)'
set ylabel 'rtt (s)'
set grid

set output 'results/newreno_rtt.png'
set title 'Newreno: rtt vs time'
plot 'results/newreno_rtt.txt' using 1:3 with lines linecolor rgb "blue" title 'Newreno rtt'

set output 'results/highspeed_rtt.png'
set title 'Highspeed: rtt vs time'
plot 'results/highspeed_rtt.txt' using 1:3 with lines linecolor rgb "red" title 'Highspeed rtt'

set output 'results/veno_rtt.png'
set title 'Veno: rtt vs time'
plot 'results/veno_rtt.txt' using 1:3 with lines linecolor rgb "dark-green" title 'Veno rtt'

set output 'results/vegas_rtt.png'
set title 'Vegas: rtt vs time'
plot 'results/vegas_rtt.txt' using 1:3 with lines linecolor rgb "purple" title 'Vegas rtt'