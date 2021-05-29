OBJS = rnuplot.o 
EXE =   rnuplot

CXX = clang++
FLAGS = -std=c++14 -fno-inline -Wall -g

COMPILE = $(CXX) $(FLAGS) -c 

all: $(EXE)

$(EXE): $(OBJS)
	$(CXX) -o $(EXE) $(OBJS)

%.o: %.cpp
	$(COMPILE)  $< 