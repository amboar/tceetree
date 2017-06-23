SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))

tceetree: $(OBJS)

clean:
	$(RM) $(OBJS) tceetree

cscope:
	find . -name '*.c' > cscope.files
	cscope -b -c -R

.PHONY: clean
