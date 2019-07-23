FLAG=-std=c++11 -lboost_filesystem -lboost_system \
		 -I /home/lua/SpringRecruit/Proc/cppjieba/include \
		 -ljsoncpp

searcher:searcher.cc
	g++ -o $@ $^ $(FLAG)

.PHONY:clean
	rm -f searcher


#.PHONY:all
#all:test libsearcher.so

#test:test.cc searcher.h searcher.cc 
#	g++ test.cc searcher.cc -o test $(FLAG)

#libsearcher.so:searcher.h searcher.cc 
#	g++ searcher.cc -shared -fPIC -o libsearcher.so $(FLAG)

