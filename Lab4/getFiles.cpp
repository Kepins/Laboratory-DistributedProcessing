#include "getFiles.h"

#include <iostream>

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




Folder* getFiles(const std::wstring& directoryPath, LPCWSTR lpApplicationName, bool isroot) {
	Folder* newFolder = new Folder;
	int64_t sumSize = 0;

	std::list < PROCESS_INFORMATION*> processesRunning;
	//loop over entries in directory
	for (const fs::directory_entry& entry : fs::directory_iterator(directoryPath)) {
		Finfo* newChild;
		if (entry.is_directory()) {
			//delegate new process to handle directory
			HANDLE hProcess;
			HANDLE hThread;
			STARTUPINFO si;
			PROCESS_INFORMATION* pi = new PROCESS_INFORMATION;
			DWORD dwProcessId = 0;
			DWORD dwThreadId = 0;
			ZeroMemory(&si, sizeof(si));
			ZeroMemory(pi, sizeof(*pi));
			BOOL bCreateProcess = NULL;

			std::wstring applicationName(lpApplicationName);
			std::wstring path = entry.path().wstring();
			std::wstring commandLine = applicationName + L" " + path +L" 0";
			

			bCreateProcess = CreateProcess(
				NULL,
				&commandLine[0],
				NULL,
				NULL,
				FALSE,
				CREATE_NEW_CONSOLE,
				NULL,
				NULL,
				&si,
				pi
			);
			processesRunning.push_back(pi);

			newChild = new Folder;
			newChild->folder = true;
			newChild->length = entry.file_size();
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

	for (PROCESS_INFORMATION* pi : processesRunning) {
		WaitForSingleObject(pi->hProcess, INFINITE);
		CloseHandle(pi->hThread);
		CloseHandle(pi->hProcess);
	}

	if (!isroot) {
		HANDLE parentHwnd = GetCurrentProcess();
		std::cout << parentHwnd;
		std::cout << "Zwracam drzewo\n";
	}

	return newFolder;
}

