#
# combiner: mapper.out reducer.out combiner.c
# 	gcc combiner.c -o combiner.out

# mapper.out : mapper.c
# 	gcc mapper.c -o mapper.out

# reducer.out : reducer.c
# 	gcc reducer.c -o reducer.out

CC = gcc
CFLAGS = -c

all:
	$(CC) mapper.c -o mapper
	$(CC) reducer.c -o reducer
	$(CC) combiner.c -o combiner

mapper: mapper.c
	$(CC) $(CFLAGS) mapper.c
	$(CC) mapper.c -o mapper

reducer: reducer.c
	$(CC) $(CFLAGS) reducer.c
	$(CC) reducer.c -o reducer

combiner: combiner.c
	$(CC) $(CFLAGS) combiner.c
	$(CC) combiner.c -o combiner
	
clean:	
	rm mapper reducer combiner