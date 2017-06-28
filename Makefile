SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))
DEPS:=$(OBJS:.o=.d)

CCAN_SRCS=ccan/strmap/strmap.c ccan/str/debug.c ccan/str/str.c \
	  ccan/ilog/ilog.c ccan/strmap/strmap.c ccan/str/debug.c \
	  ccan/str/str.c ccan/ilog/ilog.c ccan/take/take.c \
	  ccan/likely/likely.c ccan/tal/str/str.c ccan/tal/tal.c \
	  ccan/tal/talloc/talloc.c ccan/hash/hash.c ccan/talloc/talloc.c \
	  ccan/htable/htable.c ccan/list/list.c

CCAN_OBJS=$(patsubst %.c,%.o,$(CCAN_SRCS))
CCAN_DEPS=$(CCAN_OBJS:.o=.d)

CFLAGS=-I. -O2 -g -ggdb -Wall -std=gnu11

tceetree: $(CCAN_OBJS) $(OBJS)

CONFIGURATOR := tools/configurator/configurator
$(CONFIGURATOR): $(CONFIGURATOR).c
	$(PRE)$(CC) $(CCAN_CFLAGS) $(DEP_CFLAGS) $< -o $@
config.h: $(CONFIGURATOR)
	./$(CONFIGURATOR) $(CC) $(CCAN_CFLAGS) >$@.tmp && mv $@.tmp $@

$(OBJS) $(CCAN_OBJS): config.h

check: tceetree test-cscope
	cd test && ./test.sh || echo Tests failed

clean:
	$(RM) tceetree $(OBJS) $(DEPS)
	$(RM) config.h $(CCAN_OBJS) $(CONFIGURATOR) $(CCAN_DEPS)

cscope:
	find . -name '*.c' > cscope.files
	cscope -b -c -R

test-cscope:
	find test/src -name '*.c' > test/cscope.files
	cscope -b -c -R -itest/cscope.files -ftest/cscope.out

%.o: %.c
	    $(CC) -c $(CFLAGS) -MMD -o $@ $<

.PHONY: clean check
