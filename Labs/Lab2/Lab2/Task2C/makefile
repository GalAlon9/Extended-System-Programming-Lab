all: menu_map

exec: menu_map.c
	gcc -g -m32 -c -o menu_map.o menu_map.c
	gcc -g -m32  menu_map.o  -o menu_map
	

.PHONY: clean
clean:
	rm -rf ./*.o menu_map