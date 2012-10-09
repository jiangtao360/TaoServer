CC = g++
LIBS = -lpthread 
D_FLAGS = -g -Wall 
R_FLAGS = -O2 -Wall 

TARGET = taoserver
SRC = epoll.cpp tao_server.cpp netbase.cpp thread.cpp utility.cpp worker.cpp log.cpp memory_pool.cpp main.cpp

all : $(TARGET)

$(TARGET) : epoll.o tao_server.o netbase.o thread.o utility.o worker.o log.o memory_pool.o main.o
	$(CC) $(D_FLAGS) $(LIBS) -o $(TARGET) epoll.o tao_server.o netbase.o thread.o utility.o worker.o log.o memory_pool.o main.o

netbase.o : header.h utility.h log.h netbase.h netbase.cpp
	$(CC) $(D_FLAGS) -c netbase.cpp

epoll.o : header.h netbase.h utility.h log.h epoll.h epoll.cpp
	$(CC) $(D_FLAGS) -c epoll.cpp

tao_server.o : header.h struct.h netbase.h epoll.h tao_server.h utility.h log.h thread_pool.h thread_pool.cpp memory_pool.h tao_server.cpp
	$(CC) $(D_FLAGS) -c tao_server.cpp
	
thread.o : header.h log.h thread.h thread.cpp
	$(CC) $(D_FLAGS) -c thread.cpp

utility.o : header.h log.h utility.h utility.cpp
	$(CC) $(D_FLAGS) -c utility.cpp

worker.o : header.h struct.h worker.h worker.cpp
	$(CC) $(D_FLAGS) -c worker.cpp

log.o : header.h log.h log.cpp
	$(CC) $(D_FLAGS) -c log.cpp

memory_pool.o : header.h utility.h log.h memory_pool.h memory_pool.cpp
	$(CC) $(D_FLAGS) -c memory_pool.cpp

main.o : header.h utility.h log.h tao_server.h 
	$(CC) $(D_FLAGS) -c main.cpp


debug:
	$(CC) $(D_FLAGS) $(LIBS) -o $(TARGET) $(SRC)

release:
	$(CC) $(R_FLAGS) $(LIBS) -o $(TARGET) $(SRC)
	
.PHONY : clean
clean:
	rm -f *.o core* $(TARGET)


