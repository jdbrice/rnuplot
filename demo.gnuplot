set terminal pngcairo font "Helvetica, 18" size 1200,900 
set output 'demo.png'

set xlabel "𝛂" 
set ylabel "Counts"

set bars small #removes caps from error bars

# root2gnuplot input.root:h1 h1.dat "x y xlow xhigh ylow yhigh"
plot 'h1.dat' u 1:2:3:4:($5):($6) with xyerr ls 2 pt 15 ps 0 lw 2 t 'ROOT histogram'