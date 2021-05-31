# Rnuplot
Use gnuplot to directly plot ROOT data, no intermediate steps

# Usage
1. Write `gnuplot` plotting scripts like normal
2. Add a comment line in your script like this
```bash
# root2gnuplot input.root:histogram_name output.dat "x y xlow xhigh ylow yhigh"
```
this line instructs rnuplot how to convert your ROOT data. 
It uses an internal version of the `root2gnuplot` tool. See [details here](https://github.com/jdbrice/root2gnuplot). You can control the format of the conversion for custom plotting types in `gnuplot`. You can also plot 'directly' from ROOT TTrees.

3. Run your script with `rnuplot` instead of `gnuplot` and it will just work

## Full usage details
```bash
Usage:
  rnuplot [OPTION...] [FILE]

  -c, --cache             Use cached data, set to true to use cached data
  -f, --file arg          File
  -d, --debug arg         Enable debugging
  -r, --root2gnuplot arg  Convert ROOT data using root2gnuplot (useful if
                          not specified in gnuplot script comment
  -h, --help              Print usage
```

# Example
```gnuplot
set terminal pngcairo font "Helvetica, 18" size 1200,900 
set output 'demo.png'

set xlabel "𝛂" 
set ylabel "Counts"

set bars small #removes caps from error bars

# root2gnuplot input.root:histo_name h1.dat "x y xlow xhigh ylow yhigh"
plot 'h1.dat' u 1:2:3:4:($5):($6) with xyerr ls 2 pt 15 ps 0 lw 2 t 'ROOT histogram'
```
OR without modifying your gnuplot script at all you can:
```bash
rnuplot -r 'input.root:histo_name h1.dat "x y xlow xhigh ylow yhigh"' script.gnuplot
```
with a script like this:
```gnuplot
set terminal pngcairo font "Helvetica, 18" size 1200,900 
set output 'h1.png'

set xlabel "x" 
set ylabel "y"

set bars small #removes caps from error bars
set style fill   solid 1.00 border

plot 'h1.dat' u ($1+($3-$4)/2.0):2  with fillsteps fc rgb "#aa00ff" t "h1", 'h1.dat' u 1:2:3:4:($5):($6) with xyerr ls 2 pt 15 ps 0 lw 2 lc rgb "#000000" notitle
```


Notice that these are valid `gnuplot` scripts. In fact, once the conversion step is done (first time) you can run future plots with `gnuplot` directly.
Only comments with the `root2gnuplot` word are parsed for conversions

# Build Steps

1. clone the repo
2. build: `make`

## Install
just put `rnuplot` somewhere on the PATH.  
Then you are ready to use `rnuplot`, also make sure `gnuplot` is on the PATH

## Requirements
- c++14
- ROOT (tested with v6-24-00@v6-24-00)
- gnuplot (tested with v5.4) on the PATH

# Uses
- [cxxopts](https://github.com/jarro2783/cxxopts) as internal dependency

