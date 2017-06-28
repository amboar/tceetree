SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))
DEPS:=$(OBJS:.o=.d)

CFLAGS=-O2 -g -ggdb -Wall -std=gnu11

# -include $(DEPS)

tceetree: $(OBJS)

check: tceetree test-cscope
	cd test && ./test.sh || echo Tests failed

clean:
	$(RM) $(OBJS) tceetree

cscope:
	find . -name '*.c' > cscope.files
	cscope -b -c -R

test-cscope:
	find test/src -name '*.c' > test/cscope.files
	cscope -b -c -R -itest/cscope.files -ftest/cscope.out

%.o: %.c
	    $(CC) -c $(CFLAGS) -MMD -o $@ $<

.PHONY: clean check
