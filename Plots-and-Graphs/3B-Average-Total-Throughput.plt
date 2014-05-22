set terminal png
set output "3B.png"
set title "Average Throughput per Source"
set xlabel "Number of Sources"
set ylabel "Average Throughput (in Mbps)"

set xrange [0:+10]
plot "-" title "Avg Throughput per Source" with linespoints
2 1.9238
4 0.980032
5 0.777269
6 0.654918
7 0.554011 
8 0.48962
10 0.389805
e
