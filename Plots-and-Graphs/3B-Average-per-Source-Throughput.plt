set terminal png
set output "3B-Total.png"
set title "Average Total Throughput"
set xlabel "Number of Sources"
set ylabel "Average Total Throughput (in Mbps)"

set xrange [0:+10]
plot "-" title "Avg Total Throughput" with linespoints
2 3.8476
4 3.920128
6 3.929508
8 3.91696
10 3.89805 
e
