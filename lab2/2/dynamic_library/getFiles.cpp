#include "getFiles.h"



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

Finfo::~Finfo(){}

Folder::~Folder(){
	for (Finfo* child : children) {
		delete child;
	}
}




Folder* getFiles(const std::string& directoryPath) {
	Folder* newFolder = new Folder;
	__int64_t sumSize = 0;

	//loop over entries in directory
	for (const fs::directory_entry& entry : fs::directory_iterator(directoryPath)) {
		Finfo* newChild;
		if (entry.is_directory()) {
			newChild = getFiles(entry.path().string());
			sumSize += newChild->length;
		}
		else {
			newChild = new Finfo;
			newChild->folder = false;
			newChild->length = entry.file_size();
			sumSize += entry.file_size();
		}
		newChild->name = entry.path().filename().string();
		newChild->mod_time = to_time_t(entry.last_write_time());
		newChild->parent = newFolder;

		newFolder->children.insert(newFolder->children.end(), newChild);
	}
	newFolder->folder = true;
	fs::directory_entry entry(directoryPath);
	newFolder->name = entry.path().filename().string();
	newFolder->mod_time = to_time_t(entry.last_write_time());
	newFolder->length = sumSize;

	return newFolder;
}

