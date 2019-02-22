.PHONY:all
all:HttpServer upload
HttpServer:HttpServer.hpp Utils.hpp ThreadPool.hpp main.cpp
	g++ $^ -o $@ -lpthread -std=c++11
upload:UpLoad.hpp
	g++ $^ -o $@ -std=c++11
.PHONY:clean
clean:
	rm HttpServer
