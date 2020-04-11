CPP = g++
FLAGS = -g3 -Wall -pedantic
SRC_PATH = ./src
BLD_PATH = ./build

splitter: splitter.o
	$(CPP) $(FLAGS) -o $(BLD_PATH)/$@ $^

splitter.o: $(SRC_PATH)/splitter.cpp
	$(CPP) $(FLAGS) -c $^

clean:
	rm -f *.o
	rm $(BLD_PATH)/*