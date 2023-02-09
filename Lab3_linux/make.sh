g++ -c main.cpp -o main.o -std=c++17
g++ -c getFiles.cpp -o getFiles.o -std=c++17
g++ -o prog.out main.o getFiles.o -std=c++17 -lpthread
