ALL:test
OBJ = test.o task.o thread.o threadmanager.o

test:${OBJ}
	g++ --std=c++11 -o ${ALL} ${OBJ}

%.o : %.cc
	g++ --std=c++11 -c $< -o $@ -lpthread

clean:
	rm *.o
	rm ${ALL}
