main: main.cpp
	g++ main.cpp -o ../out/main `mysql_config --cflags --libs` -lpthread -ljsoncpp -g
test: test.cpp
	g++ test.cpp -o test `mysql_config --cflags --libs` -lpthread -ljsoncpp -g
