
INCDIR =.
CC=gcc
CFLAGS=-I$(INCDIR)
OBJ=socket.o crawler.o
DEPS=crawler.h socket.h
SRC=crawler.c socket.c
TARGET=crawler

$(TARGET): main.c $(OBJ) 
	$(CC) -o $(TARGET) main.c $(OBJ) $(CFLAGS)

$(OBJ): $(SRC) $(DEPS)
	$(CC) -c $(SRC) $(CFLAGS)

.PHONY: clean
clean:
	rm -f *.o *~ core $(INCDIR)/*~ $(TARGET)
