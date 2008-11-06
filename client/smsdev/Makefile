UNAME         =  $(shell uname)
MACHINE       = $(shell uname -m)

ifeq ($(UNAME),Darwin)
    CC     = c++
    LINKER = c++
    OS     = i686-apple-darwin
    THREAD = 
    THREAD_LIB =
    STATIC =
else
    CC     = g++
    LINKER = g++
    THREAD = -pthread
    THREAD_LIB = -Wl,-Bdynamic,-lpthread
    ifeq ($(UNAME),Linux)
        ifeq ($(MACHINE),x86_64)
            OS=x86_64-unknown-linux-gnu
        else
            OS=i686-pc-linux-gnu
        endif
    else
         OS=windows_intelx86
    endif
endif

BINARY   = smstest
CCFLAGS  = -g -O0 $(THREAD)
LDFLAGS  = 

RM            = rm -f
OBJEXT        = o

# Monitor Object files
OBJS =   \
         csensor.$(OBJEXT)            \
         csensor_test.$(OBJEXT)             \
         main.$(OBJEXT)  

all       : Makefile $(BINARY)
$(BINARY) : $(OBJS)
	$(LINKER) $(OBJS) $(LDFLAGS) -o $(OUT_PATH)$(BINARY)
csensor.$(OBJEXT) :
	$(CC) $(CCFLAGS) -c csensor.cpp
csensor_test.$(OBJEXT) :
	$(CC) $(CCFLAGS) -c csensor_test.cpp
main.$(OBJEXT):
	$(CC) $(CCFLAGS) -c main.cpp
clean   :
	$(RM) $(OBJS).$(OBJEXT) $(BINARY) core a.out  

      
