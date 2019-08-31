FLAG=-std=c++11 -lboost_filesystem -lboost_system \
		 -I ../../../cppjieba/include \
		 -ljsoncpp

.PHONY:all
all:libsearcher.so searcher 

#test:test.cc searcher.h searcher.cc 
#	g++ test.cc searcher.cc -o test $(FLAG)

libsearcher.so:searcher.h searcher.cc 
	g++ searcher.cc -shared -fPIC -o libsearcher.so $(FLAG)

searcher:searcher.cc
	g++ -o $@ $^ $(FLAG)

.PHONY:clean
	rm -f test libsearcher.so searcher
