CC    = gcc
CXX   = g++
INCLUDEDIR  = -I/usr/local/mysql/include
BINDIR = /usr/local/mysql/lib/plugin/

all:
	rm -f udf_qcn.so
	$(CC) $(INCLUDEDIR) -fPIC -shared -o udf_qcn.so udf_qcn.c
	sudo mv udf_qcn.so $(BINDIR)

clean:
	rm -f *.so a.out
