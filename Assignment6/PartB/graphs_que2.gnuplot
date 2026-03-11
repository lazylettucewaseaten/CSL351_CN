set terminal pngcairo size 700,600 
set xlabel 'Error Rate'
set ylabel 'Average PDR'
set yrange [0:1.1]
set grid

set output 'erbit.png'
set title 'Average PDR Vs Error Rate (Bit)'
set style line 2 lc rgb '#FFA500' lt 1 lw 3 pt 7 ps 1.5
plot 'outputdataBit.dat' with linespoints ls 2 title 'Avg PDR'

set output 'erbyte.png'
set title 'Average PDR Vs Error Rate (Byte)'
set style line 3 lc rgb '#A52A2A' lt 1 lw 3 pt 9 ps 1.5
plot 'outputdataByte.dat' with linespoints ls 3 title 'Avg PDR'

set output 'erpacket.png'
set title 'Average PDR Vs Error Rate (Packet)'
set style line 1 lc rgb '#800080' lt 1 lw 3 pt 5 ps 1.5
plot 'outputdataPacket.dat' with linespoints ls 1 title 'Avg PDR'

