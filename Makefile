CC = gcc

## all: creates executable file
all: byos.o simple-test.o
	${CC} -o simple-test $^

## %.o: compiles %.c to create a .o file
%.o: %.c
	${CC} ${CCFLAGS} -c -o $@ $<

## clean: Removes auto-generated files
.PHONY: clean
clean:
	rm -f f1 f2 f3 f4 *.o

help: Makefile
	@sed -n 's/^##//p' $<
