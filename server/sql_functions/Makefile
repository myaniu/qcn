CC    = gcc
CXX   = g++

all:
	$(CC) get_ang_dist.c -o get_ang_dist
	$(CXX) time_range.cpp -o time_range
	$(CXX) mag_dist.cpp -o mag_dist
	$(CXX) mag_dist.cpp -o mag_dist
	$(CC) -fPIC  -shared -o udf_qcn.so udf_qcn.c

julday.o:
	$(CC) julday.c ../clib/julday.c -o $(BIN)julday.1

get_ang_dist.o:
	$(CC) get_ang_dist.c -o $(BIN)get_ang_dist

time_range.o:
	$(CC) time_range.cpp -o $(BIN)get_ang_dist

mag_dist.o:
	$(CC) mag_dist.cpp -o $(BIN)mag_dist

clean:
	rm -f *.so a.out get_ang_dist time_range mag_dist
