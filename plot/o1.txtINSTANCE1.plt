 
set size ratio 1
set nokey

plot[:][:] '-' w l lt 3 lw 2, '-' w l lt 1

# bounding box
0, 0
30, 0
30, 30
0, 30
0, 0

EOF
# modules
0.00, 0.00

9, 20
25, 20
25, 30
9, 30
9, 20

0, 0
14, 0
14, 10
0, 10
0, 0

14, 0
30, 0
30, 10
14, 10
14, 0

14, 10
30, 10
30, 20
14, 20
14, 10

EOF
pause -1 'Press any key to close.'
