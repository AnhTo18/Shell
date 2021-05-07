
output: shell.o
	g++ shell.o -o output

shell.o: shell.c
	g++ -c shell.c
clean:
	rm *.o output
