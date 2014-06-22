DIR=.

CC=gcc
#LIBS=-lm

CFLAGS=$(LIBS) -Wall -Wextra

ifeq ($(DEBUG),1)
CFLAGS += -g -DDEBUG
endif

SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))

TARGET_SERVER=server
TARGET_CLIENT=client
TARGETS=$(TARGET_CLIENT) $(TARGET_SERVER)

all: $(TARGETS)
	@echo "Build all targets successfully"

$(TARGET_CLIENT): TCPEchoClient.o
	$(CC) $(CFLAGS) -o $@ $^

$(TARGET_SERVER): TCPEchoServer.o
	$(CC) $(CFLAGS) -o $@ $^

$(OBJS): %.o: %.c 
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf *.gch *.o $(TARGETS)

