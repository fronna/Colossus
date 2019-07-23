FLAG=-std=c++11 -lpthread -I /home/lua/Recruit/Proc/cppjieba/include
bin=HttpServer
cc=g++

$(bin):HttpServer.cc 
	$(cc) -o HttpServer.cc $(FLAG) #-D_DEBUG_

.PHONY:clean
clean:
	rm -f $(bin)
