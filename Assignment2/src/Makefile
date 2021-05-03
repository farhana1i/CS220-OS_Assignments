final: main.o Shell.o CommandList.o Helper.o Command.o
	g++ main.o -o main.out

main.o: main.cpp Shell.h CommandList.h Helper.h Command.h
	g++ -c main.cpp

Shell.o: Shell.h CommandList.h Helper.h Command.h
	g++ -c Shell.h

CommandList.o: CommandList.h Helper.h Command.h
	g++ -c CommandList.h

Command.o: Command.h Helper.h
	g++ -c Command.h

Helper.o: Helper.h
	g++ -c Helper.h

clean:
	rm *.o main.out
