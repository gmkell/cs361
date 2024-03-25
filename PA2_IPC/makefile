all: sales  supervisor  factory
    
sales: sales.c  wrappers.c wrappers.h  message.h  shmem.h 
	gcc -pthread  sales.c       wrappers.c             -o sales

supervisor: supervisor.c  wrappers.c  wrappers.h message.c message.h shmem.h
	gcc -pthread  supervisor.c  wrappers.c  message.c  -o supervisor

factory: factory.c  wrappers.c  wrappers.h message.c  message.h shmem.h
	gcc -pthread  factory.c     wrappers.c  message.c  -o factory

clean:
	rm -f *.o sales  factory supervisor *.log
	ipcrm -a
	rm -f /dev/shm/aboutams_*
