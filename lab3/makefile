COMPILER = gcc
TARGET = main

$(TARGET): obj/main.o obj/frame.o obj/page.o obj/pageTable.o obj/mmemory.o
	$(COMPILER) obj/main.o obj/frame.o obj/page.o obj/pageTable.o obj/mmemory.o -o $(TARGET)

obj/main.o: src/main.c
	mkdir -p obj
	$(COMPILER) -c src/main.c -o obj/main.o

obj/frame.o: src/frame.c
	mkdir -p obj
	$(COMPILER) -c src/frame.c -o obj/frame.o

obj/page.o: src/page.c
	mkdir -p obj
	$(COMPILER) -c src/page.c -o obj/page.o

obj/pageTable.o: src/pageTable.c
	mkdir -p obj
	$(COMPILER) -c src/pageTable.c -o obj/pageTable.o

obj/mmemory.o: src/mmemory.c
	mkdir -p obj
	$(COMPILER) -c src/mmemory.c -o obj/mmemory.o

clear:
	rm -rf obj $(TARGET)
