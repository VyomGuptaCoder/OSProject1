all:Admin Client

Admin:Admin.cpp
	g++ Admin.cpp -o Admin.exe -std=c++11 -lpthread

Client:Client.cpp
	g++ Client.cpp -o Client.exe -std=c++11