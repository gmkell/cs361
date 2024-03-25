all:  p1.c p2.c shmSegment.h
	gcc p1.c  wrappers.c -pthread  -o p1
	gcc p2.c  wrappers.c -pthread  -o p2

clean:
	rm -f p1 p2 