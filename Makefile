CC = gcc
CFLAGS = -Wall -I.
TARGET = auto_test
LIBRARY = libfirfilter.a
SRCS = fir_filter.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(TARGET).c $(LIBRARY)
	$(CC) $(CFLAGS) -o $@ $< -L. -lfirfilter -lm

$(LIBRARY): $(OBJS)
	ar rcs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(LIBRARY) $(OBJS)