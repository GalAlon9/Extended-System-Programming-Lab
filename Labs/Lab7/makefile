all: task1 task2 task3 task4a task4b


LineParser.o: LineParser.c
		gcc -g -m32 -Wall -c -o LineParser.o LineParser.c

task1: task1.o LineParser.o
		gcc -g -m32 -Wall -o task1 task1.o LineParser.o

task1.o: task1.c
		gcc -g -m32 -Wall -c -o task1.o task1.c

task2: task2.o
		gcc -m32 -g -Wall -o task2 task2.o 

task2.o: task2.c
		gcc -g -Wall -m32  -c -o task2.o task2.c


task3: task3.o LineParser.o
		gcc -g -m32 -Wall -o task3 task3.o LineParser.o

task3.o: task3.c
		gcc -g -m32 -Wall -c -o task3.o task3.c


task4a: task4a.o LineParser.o
		gcc -g -m32 -Wall -o task4a task4a.o LineParser.o

	
task4a.o: task4a.c
		gcc -g -m32 -Wall -c -o task4a.o task4a.c


task4b: task4b.o LineParser.o
		gcc -g -m32 -Wall -o task4b task4b.o LineParser.o

task4b.o: task4b.c
		gcc -g -m32 -Wall -c -o task4b.o task4b.c


	
.PHONY: clean
clean:
		rm -f *.o task1 task2 task3 task4a task4b