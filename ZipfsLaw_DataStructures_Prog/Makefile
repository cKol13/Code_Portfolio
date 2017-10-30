# Makefile for CSC300 PA#3 (zipf program)

# Author: Partner, Chris Kolegraf
# Class:  CSC300 Data Structures
# Date:   Fall 2015

# Usage:  make target1 target2 ...

#-----------------------------------------------------------------------

# GNU C/C++ compiler and linker:
LINK = g++

# Turn on optimization and warnings, use c++11:
CFLAGS = -std=c++11 -Wall -O2
CXXFLAGS = $(CFLAGS)

#-----------------------------------------------------------------------
# Specific targets:

# MAKE allows the use of "wildcards", to make writing compilation instructions
# a bit easier. GNU make uses $@ for the target and $^ for the dependencies.

all:	zipf

zipf:	main.o functions.o hashTable.o
	$(LINK) -o $@ $^
	
data: CXXFLAGS += -DDATA
data: zipf

time: CXXFLAGS += -DTIME
time: zipf
	
debug: CXXFLAGS += -g
debug: zipf

clean:
	rm -f *.o *~ *.wrd *.csv *.data .nfs* core zipf graph

remake: clean all
