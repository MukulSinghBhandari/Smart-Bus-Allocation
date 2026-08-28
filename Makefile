CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -I../C

TARGET = c_engine.exe
SRCS = main.c graph.c minheap.c dijkstra.c trie.c bus.c loader.c output.c database.c
OBJS = $(SRCS:.c=.o)
SQLITE_OBJ = sqlite3.o

all: $(TARGET)

$(TARGET): $(OBJS) $(SQLITE_OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(SQLITE_OBJ)

$(SQLITE_OBJ): ../C/sqlite3.c
	$(CC) -std=c11 -c ../C/sqlite3.c -o $(SQLITE_OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	del /Q $(OBJS) $(SQLITE_OBJ) $(TARGET) 2>nul || rm -f $(OBJS) $(SQLITE_OBJ) $(TARGET)

.PHONY: all clean
