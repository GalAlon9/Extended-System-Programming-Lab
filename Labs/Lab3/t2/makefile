all: exec

exec: task2b.c
	gcc -g -m32 -c -o task2b.o task2b.c
	gcc -g -m32  task2b.o  -o task2b
	

.PHONY: clean
clean:
	rm -rf ./*.o task2b
