assembler: main.o globals.o externals.o preassembler.o firstsweep.o secondsweep.o
	gcc -ansi -Wall -g main.o globals.o externals.o preassembler.o firstsweep.o secondsweep.o -o assembler -lm

main.o: main.c main.h constants.h globals.h preassembler.h firstsweep.h secondsweep.h
	gcc -c -ansi -Wall main.c -o main.o

globals.o: globals.c globals.h
	gcc -c -ansi -Wall globals.c -o globals.o

externals.o: externals.c externals.h main.h constants.h globals.h preassembler.h firstsweep.h secondsweep.h
	gcc -c -ansi -Wall externals.c -o externals.o

preassembler.o: preassembler.c main.h constants.h globals.h preassembler.h
	gcc -c -ansi -Wall preassembler.c -o preassembler.o

firstsweep.o: firstsweep.c main.h constants.h globals.h firstsweep.h
	gcc -c -ansi -Wall firstsweep.c -o firstsweep.o

secondsweep.o: secondsweep.c main.h constants.h globals.h firstsweep.h
	gcc -c -ansi -Wall secondsweep.c -o secondsweep.o -lm

