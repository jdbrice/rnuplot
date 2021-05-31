OBJS = rnuplot.o 
EXE =   rnuplot

ROOTCFLAGS    = $(shell root-config --cflags)
ROOTLIBS      = $(shell root-config --libs)
ROOTGLIBS     = $(shell root-config --glibs)

INCFLAGS = -I$(ROOTSYS)/include
LDFLAGS = -L$(ROOTSYS)/lib

CXX = clang++
FLAGS = -std=c++14 -fno-inline -Wall -g $(INCFLAGS) $(LDFLAGS)

COMPILE = $(CXX) $(FLAGS) -c 

all: $(EXE)

$(EXE): $(OBJS)
	$(CXX) -lpthread  -o $(EXE) $(OBJS) $(ROOTFLAGS) $(ROOTLIBS)

rnuplot.o: rnuplot.cpp root2gnuplot.h
	$(COMPILE)  $< 

clean:
	rm *.o
	rm rnuplot

cdemo:
	rm h1.dat
	rm demo.png

demo:
	./rnuplot demo.gnuplot