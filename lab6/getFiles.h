#ifndef GETFILES_H
#define GETFILES_H

#include <string>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <list>
#include <Windows.h>



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

	std::string serialize();
	Finfo();
	Finfo(std::string& stringSerialization, Folder* parent = nullptr);
	virtual ~Finfo();
};

class Folder : public Finfo {
public:
	std::list<Finfo*> children;

	std::string serialize();
	Folder();
	Folder(std::string& stringSerialization, Folder* parent = nullptr);
	~Folder();
};


Folder* getFiles(const std::string& directoryPath, LPCWSTR lpApplicationName, u_short portParent = 0);


#endif



