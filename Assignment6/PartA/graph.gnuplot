set terminal png size 900,600
set datafile separator whitespace
set grid
set key outside
set xlabel "Flow ID"


set output "flowthroughput.png"
set title "Flow vs Throughput"
set ylabel "Throughput (Kbps)"
plot "flowresults.dat" using 1:2 with linespoints lw 2.5 pt 7 ps 1.2 lc rgb "#1f77b4" title "Throughput"

set output "flowgoodput.png"
set title "Flow vs Goodput"
set ylabel "Goodput (Kbps)"
plot "flowresults.dat" using 1:3 with linespoints lw 2.5 pt 7 ps 1.2 lc rgb "#2ca02c" title "Goodput"

set output "flowloss.png"
set title "Flow vs Total Packets Lost"
set ylabel "Packets Lost"
plot "flowresults.dat" using 1:4 with linespoints lw 2.5 pt 7 ps 1.2 lc rgb "#d62728" title "Packets Lost"







