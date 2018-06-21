set terminal png
set xlabel "Time"
set ylabel "Concentration"
set output "comp.png"
plot "a" using 1:2 title "A" with lines 1, "a" using 1:3 title "B" with lines 2, "b" using 1:2 title "A'" with points 1, "b" using 1:3 title "B'" with points 2

