g++ -std=c++17 -c getFiles.cpp -o getFiles.o -fpic
g++ -shared -o lib_getFiles.so getFiles.o
sudo cp lib_getFiles.so /usr/lib

