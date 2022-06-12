 
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

16, 16
22, 16
22, 22
16, 22
16, 16

5, 5
11, 5
11, 11
5, 11
5, 5

5, 16
11, 16
11, 22
5, 22
5, 16

EOF
pause -1 'Press any key to close.'
