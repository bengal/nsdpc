
TARGET  := nsdpc
CFLAGS  := -Wall -pedantic

SOURCES := $(wildcard *.c)
OBJS    := $(SOURCES:.c=.o)
DEPS    := $(SOURCES:.c=.d)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

.PHONY: clean

clean:
	$(RM) $(OBJS) $(DEPS) $(TARGET)

-include $(DEPS)