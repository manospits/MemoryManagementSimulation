PTHREAD = -pthread
CC = g++
WALL = -Wall
SOURCES = generator.cc memory_management.cc process_mem_sim.cc
HEADERS = generator.h memory_management.h
FLAGS = ${Wall} ${PTHREAD}

mem_sim: process_mem_sim.o
	${CC} $^ memory_management.o generator.o ${FLAGS} -o $@

process_mem_sim.o: memory_management.o
	${CC} -c process_mem_sim.cc -o $@

memory_management.o : generator.o
	${CC} -c memory_management.cc -o $@

generator.o: generator.cc
	${CC} -c $^ -o $@

clean:
	\rm -f generator.o memory_management.o process_mem_sim.o mem_sim

count:
	wc ${SOURCES} ${HEADERS}
