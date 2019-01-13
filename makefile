IDIR = include
CC = gcc
CFLAGS = -I$(IDIR) `pkg-config fuse3 libarchive --cflags` -D_FILE_OFFSET_BITS=64
LIBS = `pkg-config fuse3 libarchive --libs` -Wall
DEBUG_FLAGS = -O0 -g

SRCDIR = src
SRCS = $(SRCDIR)/filesystem.c $(SRCDIR)/mount.c $(SRCDIR)/main.c

OBJDIR = obj
OBJS = $(patsubst $(SRCDIR)%, $(OBJDIR)%, $(SRCS:.c=.o))

INCLUDES = filesystem.h mount.h definitions.h
DEPS = $(patsubst %,$(IDIR)/%,$(INCLUDES))

EXECUTABLE = bin/archmount

.PHONY: clean test u make-dirs

all: make-dirs $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(OBJS) $(LIBS) $(DEBUG_FLAGS)
	@echo "Compiled succesifully"

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEPS)
		$(CC) -c -o $@ $< $(CFLAGS) $(DEBUG_FLAGS)

install: all
	mv ./bin/archmount /usr/bin

make-dirs:
	mkdir -p obj
	mkdir -p bin

clean:
	rm $(OBJS)

test:
	./bin/archmount ./bin/mnt ./bin/gtk.tar.xz

u:
	fusermount -u ./bin/mnt
