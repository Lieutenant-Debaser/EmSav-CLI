EmSav: ioHandle.o savStructure.o src/main.cpp
	g++ -std=c++11 ioHandle.o savStructure.o src/main.cpp -o build/EmSav

ioHandle.o: src/ioHandle.h src/ioHandle.cpp
	g++ -c -std=c++11 src/ioHandle.cpp

savStructure.o: src/savStructure.h src/savStructure.cpp
	g++ -c -std=c++11 src/savStructure.cpp

clean:
	rm *.o

redo:
	rm *.o
	make savStructure.o
	make ioHandle.o
	make EmSav

execute:
	cd build && ./EmSav
