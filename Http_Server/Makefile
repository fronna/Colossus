bin=HttpServer
cc=g++ -std=c++11
LDFLAGS=-lpthread -I /home/lua/Proc/ColossusProject/cppjieba/include

$(bin):HttpServer.cc ../Search_Engine/searcher/libsearcher.so 
	$(cc) -o $@ $^ ../Search_Engine/searcher/libsearcher.so  $(LDFLAGS) #-D_DEBUG_
#testCgi:testCgi.cc 
#	$(cc) -o $@ $^

.PHONY:clean
clean:
	rm -f $(bin) testCgi
