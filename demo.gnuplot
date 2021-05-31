set terminal pngcairo font "Helvetica, 18" size 1200,900 
set output 'demo.png'

set xlabel "𝛂" 
set ylabel "Counts"

set bars small #removes caps from error bars

# root3gnuplot input.root:h1 h1.dat "x y xlow xhigh ylow yhigh"
plot 'h1.dat' u 1:2:3:4:($5):($6) with xyerr ls 2 pt 15 ps 0 lw 2 t 'ROOT histogram'

set style fill   solid 1.00 border
# Example for plotting directly from a TTree named 'Stg'
# root2gnuplot "tree.root:Stg{pt[rtid]>> h1(500, 0, 5.0)}{rtid>0}" rptid.dat "x y xlow xhigh ylow yhigh"
plot 'rptid.dat' u ($1+($3-$4)/2.0):2  with fillsteps fc rgb "#aa00ff", 'rptid.dat' u 1:2:3:4:($5):($6) with xyerr ls 2 pt 15 ps 0 lw 2 lc rgb "#000000" t 'ROOT histogram'
