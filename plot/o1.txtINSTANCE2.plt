 
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

16, 0
23, 0
23, 15
16, 15
16, 0

14, 15
30, 15
30, 30
14, 30
14, 15

2, 15
14, 15
14, 30
2, 30
2, 15

23, 0
30, 0
30, 15
23, 15
23, 0

EOF
pause -1 'Press any key to close.'
