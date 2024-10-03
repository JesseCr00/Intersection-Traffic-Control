# Makefile for 3143 Assignment 2
all: traffic
traffic: traffic_control.cpp
	mpic++ -g -fsanitize=address traffic_control.cpp -Wall -fopenmp -o traffic_control
	mpirun -np 4 traffic_control 4
clean:
	rm *.o traffic_control