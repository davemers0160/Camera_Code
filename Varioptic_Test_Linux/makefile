# Makefile for compiling code for the Varioptic lens test
# Created: 21 Oct 2015
# David Emerson

# Compiler Variable to indicate which compiler to use
CXX = g++

# Compiler flags 
CXXFLAGS = -c -Wall

# start of all compiling efforts
all: var_test

var_test: lens_test_main.o Varioptic_Class.o
	$(CXX) lens_test_main.o Varioptic_Class.o -o var_test

lens_test_main.o: lens_test_main.cpp
	$(CXX) $(CXXFLAGS) lens_test_main.cpp

Varioptic_Class.o: Varioptic_Class.cpp
	$(CXX) $(CXXFLAGS) Varioptic_Class.cpp

# Clean the object files and executable
clean:
    rm *o var_test
