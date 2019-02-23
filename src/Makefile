.PHONY:all
all:httpserver upload
httpserver:HttpServer.hpp Utils.hpp ThreadPool.hpp main.cpp
	g++ $^ -o $@ -lpthread -std=c++11
upload:TestUpLoad.cpp
	g++ $^ -o $@ -std=c++11
.PHONY:clean
clean:
	rm httpserver upload
