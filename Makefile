CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -g

# Use pkg-config if raylib provides it, otherwise fall back to manual flags
RAYLIB_CFLAGS := $(shell pkg-config --cflags raylib 2>/dev/null)
RAYLIB_LIBS   := $(shell pkg-config --libs raylib 2>/dev/null)
ifeq ($(RAYLIB_LIBS),)
    RAYLIB_LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
endif

SERVER_SRC = server.c
CLIENT_SRC = client.c
HEADERS    = common.h

all: server client

server: $(SERVER_SRC) $(HEADERS)
	$(CC) $(CFLAGS) $(RAYLIB_CFLAGS) -o $@ $(SERVER_SRC) -lm

client: $(CLIENT_SRC) $(HEADERS)
	$(CC) $(CFLAGS) $(RAYLIB_CFLAGS) -o $@ $(CLIENT_SRC) $(RAYLIB_LIBS)

run-server: server
	./server

run-client: client
	./client

clean:
	rm -f server client

.PHONY: all clean run-server run-client
