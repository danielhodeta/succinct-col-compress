CPP = g++
FLAGS = -g3 -Wall -pedantic
SRC_PATH = ./src
BLD_PATH = ./build

split: split.o
	$(CPP) $(FLAGS) -o $(BLD_PATH)/$@ $^

split.o: $(SRC_PATH)/split.cpp
	$(CPP) $(FLAGS) -c $^

clean:
	rm -f *.o
	rm $(BLD_PATH)/*