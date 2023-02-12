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
	tree_access_t* tree_access;
	Folder* newFolder = nullptr;
};

void* run_thread(void* args){
	thread_data* t_data = (thread_data*) args;
	getFiles(t_data->directoryPath, t_data->tree_access, t_data->newFolder);
	pthread_exit(NULL);
}

void getFiles(const std::string& directoryPath, tree_access_t* tree_access, Folder* newFolder) {
	int64_t sumSize = 0;
	
	
	bool try_insert = true;
	while(try_insert){
		sem_wait(&tree_access->sem);
		if(tree_access->writable){
			newFolder->folder = true;
			fs::directory_entry entry(directoryPath);
			newFolder->name = entry.path().filename().string();
			newFolder->mod_time = to_time_t(entry.last_write_time());
			
			if(newFolder->parent != nullptr){
				newFolder->parent->children.insert(newFolder->parent->children.end(), newFolder);
			}
			tree_access->writable = false;
			try_insert = false;
		}
		sem_post(&tree_access->sem);
	}
	
	
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
			t_data->tree_access = tree_access;
			t_data->newFolder = new Folder;
			t_data->newFolder->parent = newFolder;
			
			sem_wait(&tree_access->sem_threads_left);
			tree_access->threads_left += 1;
			sem_post(&tree_access->sem_threads_left);
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

			
			
			bool try_insert = true;
			while(try_insert){
				sem_wait(&tree_access->sem);
				if(tree_access->writable){
					newFolder->children.insert(newFolder->children.end(), newChild);
					tree_access->writable = false;
					try_insert = false;
				}
				sem_post(&tree_access->sem);
			}
		}
		
	}
	for(pthread_t* thread : threads){
		pthread_join(*thread, NULL);
		delete thread;
	}
	

	newFolder->length = sumSize;
	
	for(thread_data* t_data : threads_data){
		delete t_data;
	}
	
	sem_wait(&tree_access->sem_threads_left);
	tree_access->threads_left -= 1;
	sem_post(&tree_access->sem_threads_left);
}


void writeFiles(const Folder* root) {
	//indentation is used to represent tree structure of folders
	static std::string indentation = "";

	//print current folder name
	std::cout << indentation << (Finfo*)root << std::endl;

	//every call adds one '-' to indentation
	indentation += "-";

	//print information of every child
	for (Finfo* child : root->children) {

		if (child->folder) {
			writeFiles((Folder*)child);
		}

		else {
			std::cout << indentation << child << std::endl;
		}
	}

	//removes one '-' from indentation
	indentation.pop_back();
}



void writer(const std::string& directoryPath){
	Folder* root = new Folder;
	
	tree_access_t tree_access;
	tree_access.writable = true;
	tree_access.threads_left = 1;
	sem_init(&tree_access.sem, 0, 1);
	sem_init(&tree_access.sem_threads_left, 0, 1);
	
	pthread_t* thread = new pthread_t;
	thread_data* t_data = new thread_data;
	t_data->directoryPath = directoryPath;
	t_data->tree_access = &tree_access;
	t_data->newFolder = root;
	pthread_create(thread, NULL, run_thread, t_data);

	bool loop = true;
	while(loop){
		sem_wait(&tree_access.sem);
		if(!tree_access.writable){
			writeFiles(root);
			std::cout<<std::endl;
			tree_access.writable = true;
			sleep(1);
		}
		sem_post(&tree_access.sem);
		sem_wait(&tree_access.sem_threads_left);
		if(!tree_access.threads_left){
			loop = false;
		}
		sem_post(&tree_access.sem_threads_left);
		
	}

	
	
	sem_destroy(&tree_access.sem);
	sem_destroy(&tree_access.sem_threads_left);
	delete root;
	delete thread;
}

