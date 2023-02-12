#include "getFiles.h"

#include <iostream>

# define READ 0
# define WRITE 1

//used for casting std::filesystem::file_time_type to std::time_t
std::time_t to_time_t(std::filesystem::file_time_type tp)
{
	using namespace std::chrono;
	system_clock::time_point sctp = time_point_cast<system_clock::duration>(tp - std::filesystem::file_time_type::clock::now() + system_clock::now());
	return system_clock::to_time_t(sctp);
}

std::ostream& operator<<(std::ostream& out, Finfo* filePtr){
	//std::gmtime in VS needs to have a security warning disabled
	std::tm* gmt = std::gmtime(&(filePtr->mod_time));
	out << filePtr->name << "(folder: " << filePtr->folder << ", size: "<<filePtr->length
		<<", mod_time: "<< std::put_time(gmt, "%A, %d %B %Y %H:%M") << ")";
	return out;
}

//
//Finfo
//

Finfo::Finfo() {}

std::string Finfo::serialize() {
	std::string result = "<Finfo>";
	if (folder)
		result += "<is_folder>1</is_folder>";
	else
		result += "<is_folder>0</is_folder>";
	result += "<name>" + name + "</name>";
	result += "<length>" + std::to_string(length) + "</length>";
	result += "<mod_time>" + std::to_string(mod_time) + "</mod_time>";
	result += "</Finfo>";
	return result;
}

Finfo::Finfo(std::string& str, Folder* parent) {
	std::string field = "<is_folder>";
	std::string endField = "</is_folder>";
	folder = atoi(&str[str.find(field) + field.length()]);
	field = "<name>";
	endField = "</name>";
	int beggining = str.find(field) + field.length();
	int length = str.find(endField) - beggining;
	this->name = str.substr(beggining, length);

	field = "<length>";
	endField = "</length>";
	beggining = str.find(field) + field.length();
	length = str.find(endField) - beggining;
	std::string string_size = str.substr(beggining, length);
	//size of this file
	this->length = atoi(string_size.c_str());

	field = "<mod_time>";
	endField = "</mod_time>";
	beggining = str.find(field) + field.length();
	length = str.find(endField) - beggining;
	std::string string_mod_time = str.substr(beggining, length);
	this->mod_time = time_t(atoi(string_mod_time.c_str()));

	//since the pointer changes we can not read it form the string
	this->parent = parent;

	//erases all the info that was used
	str.erase(0, str.find("</Finfo>") + std::string("</Finfo>").length());
}

Finfo::

Finfo::~Finfo() {}

//
//Folder
//
Folder::Folder() {}

std::string Folder::serialize() {
	std::string result = "<Folder>" + Finfo::serialize();
	result += "<children>";
	for (Finfo* child : children) {
		if (child->folder) {
			result += ((Folder*)child)->serialize();
		}
		else {
			result += child->serialize();
		}
	}
	result += "</children>";
	result += "</Folder>";
	return result;
}

std::string& beforeFinfo(std::string& str){
	//erases "<Folder>"
	str.erase(0, std::string("<Folder>").length());

	return str;
}

Folder::Folder(std::string& str, Folder* parent) : Finfo(beforeFinfo(str), parent) {
	
	//consturctor erases "<Finfo> ... </Finfo>"
	

	this->parent = parent;

	//erases "<children>"
	str.erase(0, std::string("<children>").length());

	//check first if first characters match "</children>"
	while (str.substr(0, std::string("</children>").length()) != "</children>") {
		//check first if first characters match "<Finfo>"
		if (str.substr(0, std::string("<Finfo>").length()) == "<Finfo>"){
			Finfo* newChild = new Finfo(str, this);
			//consturctor erases "<Finfo> ... </Finfo>"
			this->children.push_back(newChild);
		}
		//first characters must match "<Folder>"
		else {
			Folder* newChild = new Folder(str, this);
			//consturctor erases "<Folder> ... </Folder>"
			this->children.push_back(newChild);
		}
	}
	str.erase(0, std::string("</children>").length());
	str.erase(0, std::string("</Folder>").length());
}

Folder::~Folder(){
	for (Finfo* child : children) {
		delete child;
	}
}




Folder* getFiles(const std::string& directoryPath) {
	int child = 0;
	Folder* newFolder = new Folder;
	int64_t sumSize = 0;
	
	int pipeParent = -1;
	int semPipeParent = -1;
	std::vector<int> pipes;
	std::vector<int> semaphorPipes;
	//loop over entries in directory
	for (const fs::directory_entry& entry : fs::directory_iterator(directoryPath)) {
		Finfo* newChild;
		if (entry.is_directory()) {
			int pipeDir[2];
			int semDir[2];
			pipe(semDir);
			pipe(pipeDir); 
			if(fork() == 0){
				close(pipeDir[READ]);
				close(semDir[READ]);
				pipeParent = pipeDir[WRITE];
				semPipeParent = semDir[WRITE];
				child = 1;
				delete newFolder;
				newFolder = getFiles(entry.path().string());
				break;
			}
			close(pipeDir[WRITE]);
			close(semDir[WRITE]);
			pipes.push_back(pipeDir[READ]);
			semaphorPipes.push_back(semDir[READ]);
		}
		else {
			newChild = new Finfo;
			newChild->folder = false;
			newChild->length = entry.file_size();
			sumSize += entry.file_size();
			newChild->name = entry.path().filename().string();
			newChild->mod_time = to_time_t(entry.last_write_time());
			newChild->parent = newFolder;
			newFolder->children.insert(newFolder->children.end(), newChild);
		}
	}
	
	if(child){
		
		std::string ser = newFolder->serialize();
		write(pipeParent, ser.c_str(), ser.length());
		write(semPipeParent, "1", 2);
		close(pipeParent);
		close(semPipeParent);
		delete newFolder;
		return NULL;
	}
	//wait for info from children
	for(int i=0;i<pipes.size();i++){
			int pipeChild = pipes[i];
			int semChild = semaphorPipes[i];
			
			//this number should be much bigger for arbitrary folders
			const int bufferSize = 50000;
			char buffer[bufferSize] = "0";
			
			while(true){
				read(semChild, buffer,bufferSize);
				if(std::string(buffer) == "1")
				{
					break;
				}
			}
			
			read(pipeChild, buffer, bufferSize);
			
			close(pipeChild);
			close(semChild);
			std::string bufferstr = std::string(buffer);
			Folder* newChild = new Folder(bufferstr, newFolder);
			newFolder->children.insert(newFolder->children.end(), newChild);
	}
	
	
	newFolder->folder = true;
	fs::directory_entry entry(directoryPath);
	newFolder->name = entry.path().filename().string();
	newFolder->mod_time = to_time_t(entry.last_write_time());
	newFolder->length = sumSize;

	return newFolder;
}

