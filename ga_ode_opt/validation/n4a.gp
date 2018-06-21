set terminal png
set xlabel "Time"
set ylabel "Concentration"
set output "comp.png"
plot "a" using 1:2 title "A" with lines 1, "a" using 1:3 title "B" with lines 2,  "a" using 1:4 title "C" with lines 3,  "a" using 1:5 title "D" with lines 4,  "a" using 1:6 title "E" with lines 5,  "a" using 1:7 title "F" with lines 6, "b" using 1:2 title "A'" with points 1, "b" using 1:3 title "B'" with points 2, "b" using 1:4 title "C'" with points 3, "b" using 1:5 title "D'" with points 4, "b" using 1:6 title "E'" with points 5, "b" using 1:7 title "F'" with points 6

