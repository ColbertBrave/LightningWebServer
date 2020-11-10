加上-c和不加-c，有时候不加能运行，有时候不加不能运行
不加不能运行的情况
```
test: Main.o Logging.o
	g++ Main.cpp Logging.cpp -o test -lpthread 

Main.o: Main.cpp
	g++ -c Main.cpp -o Main.o

Logging.o: Logging.cpp
	g++ -c Logging.cpp -o Logging.o -lpthread
```

不加时会报错undefined reference to main()