C    = c++
BIN  = ../bin/


all:
	$(C) get_ang_dist.c -o get_ang_dist
	$(C) time_range.cpp -o time_range
	$(C) mag_dist.cpp -o mag_dist

julday.o:
	$(C) julday.c ../clib/julday.c -o $(BIN)julday.1

get_ang_dist.o:
	$(C) get_ang_dist.c -o $(BIN)get_ang_dist

time_range.o:
	$(C) time_range.cpp -o $(BIN)get_ang_dist

mag_dist.o:
	$(C) mag_dist.cpp -o $(BIN)mag_dist

clean:
	rm -f a.out get_ang_dist time_range mag_dist
