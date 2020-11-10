test: Main.o Logging.o
	g++ -g Main.cpp Logging.cpp -g -o test -lpthread 
	#g++ Main.cpp Logging.cpp -o test -lpthread

Main.o: Main.cpp
	g++ -c Main.cpp -o Main.o

Logging.o: Logging.cpp
	g++ -c Logging.cpp -o Logging.o -lpthread

#ubuntu下-lpthread要加到最后