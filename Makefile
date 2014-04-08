
CPP	=	g++ -O3  -std=c++11 -fexceptions -fpermissive
INCPATH  =
LIBPATH = 
SYSLIBS = 	-lstdc++ -lboost_regex -lboost_thread -lboost_system -lrt  -lpthread
PRGS	=	blarghd

all: le_build blarghd


blarghd: le_build/blargh.o
	$(CPP) -fexceptions -D_BOOL  $^ -I$(INCPATH) -L$(LIBPATH) $(SYSLIBS) -o $@

clean:
	rm -rf le_build blarghd;\

le_build/blargh.o: blargh.cpp blargh.h restfoo.h
	$(CPP) -fexceptions -D_BOOL -c $< -I$(INCPATH) -L$(LIBPATH)  -DHAVE_IOMANIP -DHAVE_IOSTREAM -DHAVE_LIMITS_H -o $@

le_build:
	mkdir ./le_build












