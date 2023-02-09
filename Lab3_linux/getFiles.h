#ifndef GETFILES_H
#define GETFILES_H

#include <string>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <list>
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>


namespace fs = std::filesystem;

class Folder;

class Finfo {
public:
	bool folder;
	std::string name;
	__int64_t length;
	std::time_t mod_time;
	Folder* parent = nullptr;
	friend std::ostream& operator<<(std::ostream& out, Finfo* filePtr) ;
	virtual ~Finfo();
};

class Folder : public Finfo {
public:
	std::list<Finfo*> children;
	~Folder();
};


Folder* getFiles(const std::string& directoryPath);


#endif



