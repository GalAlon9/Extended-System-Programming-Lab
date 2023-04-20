all: exec

exec: task1c.c
	gcc -g -m32 -c -o task1c.o task1c.c
	gcc -g -m32  task1c.o  -o task1c
	

.PHONY: clean
clean:
	rm -rf ./*.o task1c
