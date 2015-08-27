CC = gcc
CFLAGS = -Os -Iduktape -Ilibuv/include
LDFLAGS = -lm -lpthread ./libuv.a
OBJS = src/main.o duktape/duktape.o
EXEC = dukio

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) -o $(EXEC) $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	-rm $(OBJS)
	-rm $(EXEC)
