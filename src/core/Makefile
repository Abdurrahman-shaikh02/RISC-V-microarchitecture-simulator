CC = gcc
CFLAGS = -g -lm
LDLIBS = -lm

SRC = control.c alu.c ex.c id.c if.c logger.c ma.c main.c memory.c utility.c wb.c branch_prediction.c static_prediction.c dynamic_prediction.c stats.c cache_main.c cache_read.c cache_write.c cache_setters.c cache_eviction.c configure.c

OBJ = $(SRC:.c=.o)

TARGET = a.out

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
	rm log.log

cache:
	cc stats.c cache_main.c cache_eviction.c cache_read.c cache_write.c cache_setters.c logger.c cachesim.c -lm	-o cachesim


