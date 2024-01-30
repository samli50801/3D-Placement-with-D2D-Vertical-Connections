reset
set tics
unset key
set title "The result of Cell Legalization"
set size 1, 1
set object 1 rect from 0,0 to 30,30 fc rgb 'black'
set object 2 rect from 0,10 to 16,20 fs empty border fc rgb 'green'
set object 3 rect from 16,10 to 30,20 fs empty border fc rgb 'green'
set object 4 rect from 5,0 to 21,10 fs empty border fc rgb 'green'
set object 5 rect from 11,20 to 27,30 fs empty border fc rgb 'green'
set style line 1 lc rgb "red" lw 3
set border ls 1
set terminal png
set output "gnuplot/cellLegalize.png"
plot [0:30][0:30]'gnuplot/line' w l lt 2 lw 1
set terminal x11 persist
replot
exit