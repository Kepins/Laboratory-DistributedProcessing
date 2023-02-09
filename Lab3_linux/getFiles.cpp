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

struct thread_data{
	std::string directoryPath;
	Folder* result = nullptr;
};

void* run_thread(void* args){
	thread_data* t_data = (thread_data*) args;
	pid_t tid = syscall(SYS_gettid);
	std::stringstream msg;
	msg<< "Thread "<<tid<<" started proccessing "<< t_data->directoryPath<<std::endl;
	std::cout<< msg.str();
	t_data->result = getFiles(t_data->directoryPath);
	pthread_exit(NULL);
}

Folder* getFiles(const std::string& directoryPath) {
	Folder* newFolder = new Folder;
	__int64_t sumSize = 0;
	std::list<pthread_t*> threads;
	std::list<thread_data*> threads_data;
	//loop over entries in directory
	for (const fs::directory_entry& entry : fs::directory_iterator(directoryPath)) {
		Finfo* newChild;
		if (entry.is_directory()) {
			pthread_t* thread = new pthread_t;
			threads.push_back(thread);
			thread_data* t_data = new thread_data;
			threads_data.push_back(t_data);
			t_data->directoryPath = entry.path().string();
			pthread_create(thread, NULL, run_thread, t_data);
			
			
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
	for(pthread_t* thread : threads){
		pthread_join(*thread, NULL);
		delete thread;
	}
	for(thread_data* t_data : threads_data){
		Folder* newChild = t_data->result;
		sumSize += newChild->length;
		newChild->parent = newFolder;
		
		newFolder->children.insert(newFolder->children.end(), newChild);
		delete t_data;
	}
	
	
	newFolder->folder = true;
	fs::directory_entry entry(directoryPath);
	newFolder->name = entry.path().filename().string();
	newFolder->mod_time = to_time_t(entry.last_write_time());
	newFolder->length = sumSize;
	
	return newFolder;
}

