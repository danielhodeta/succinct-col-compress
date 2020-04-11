CPP = g++
FLAGS = -g3 -Wall -pedantic
SRC_PATH = ./src
BLD_PATH = ./build

Task1: Task1.o
	$(CPP) $(FLAGS) -o $(BLD_PATH)/$@ $^

Task1.o: $(SRC_PATH)/Task1.cpp
	$(CPP) $(FLAGS) -c $^

clean:
	rm -f *.o
	rm $(BLD_PATH)/*