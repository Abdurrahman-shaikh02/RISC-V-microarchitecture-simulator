CC = gcc
CFLAGS = -g

SRC = control.c alu.c ex.c id.c if.c logger.c ma.c main.c memory.c utility.c wb.c branch_prediction.c static_prediction.c dynamic_prediction.c

OBJ = $(SRC:.c=.o)

TARGET = a.out

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
	rm log.log

cache:
	cc cache_main.c cache_eviction.c cache_read.c cache_write.c cache_setters.c logger.c cache_test.c -lm


