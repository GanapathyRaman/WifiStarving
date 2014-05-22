set terminal png
set output "3A.png"
set title "Collision Probability Plot"
set xlabel "Number of Sources"
set ylabel "Collision Probability"

set xrange [0:+10]
plot "-" title "p" with linespoints
2 0.057072
4 0.142836
6 0.195476
8 0.238880
10 0.274693
e
