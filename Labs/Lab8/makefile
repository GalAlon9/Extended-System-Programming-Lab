all: task1 task4

task1: task1.o
	gcc -m32 -o task1 task1.o

task1.o: task1.c
	gcc -m32 -c -o task1.o task1.c

task4: task4.o
	gcc -m32 -fno-pie -o task4 task4.o

task4.o: task4.c
	gcc -m32 -fno-pie -c -o task4.o task4.c

.PHONY: clean
clean:
	rm -f *.o task1 task4
