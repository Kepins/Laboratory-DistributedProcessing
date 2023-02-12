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
#include <semaphore.h>


namespace fs = std::filesystem;

class Folder;

class Finfo {
public:
	bool folder;
	std::string name;
	int64_t length;
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

struct tree_access_t {
	sem_t sem;
	bool writable;
	sem_t sem_threads_left;
	int threads_left;
};

void getFiles(const std::string& directoryPath, tree_access_t* tree_access, Folder* newFolder);

void writer(const std::string& directoryPath);

#endif



