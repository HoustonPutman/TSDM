CC=g++

CFLAGS=-Wall -std=c++1y

#TRACE=401.bzip2
#TRACE=471.omnetpp
TRACE=alias_test
#TRACE=436.cactusADM
#TRACE=473.astar
TRACE_END=.L2.trace

DATA=../data/

CACHE_SIZE=2048

ASSOC=16

all: run

clean:
	rm -f cache_sim


#test:
#	gdb --args ./sdm $(DATA_FILE)

run:
	./cache_sim $(DATA)$(TRACE)$(TRACE_END) $(CACHE_SIZE) $(ASSOC) 0

compile:
	$(CC) $(CFLAGS) -o cache_sim *.cpp $(ADD_FILES)
