SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))
DEPS:=$(OBJS:.o=.d)

CFLAGS=-O2 -g -ggdb -Wall -std=gnu11

# -include $(DEPS)

tceetree: $(OBJS)

clean:
	$(RM) $(OBJS) tceetree

cscope:
	find . -name '*.c' > cscope.files
	cscope -b -c -R

%.o: %.c
	    $(CC) -c $(CFLAGS) -MMD -o $@ $<

.PHONY: clean
