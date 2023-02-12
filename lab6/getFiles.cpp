#include "getFiles.h"

#include <iostream>

//used for casting std::filesystem::file_time_type to std::time_t
std::time_t to_time_t(std::filesystem::file_time_type tp)
{
	using namespace std::chrono;
	system_clock::time_point sctp = time_point_cast<system_clock::duration>(tp - std::filesystem::file_time_type::clock::now() + system_clock::now());
	return system_clock::to_time_t(sctp);
}

std::ostream& operator<<(std::ostream& out, Finfo* filePtr) {
	//std::gmtime in VS needs to have a security warning disabled
	std::tm* gmt = std::gmtime(&(filePtr->mod_time));
	out << filePtr->name << "(folder: " << filePtr->folder << ", size: " << filePtr->length
		<< ", mod_time: " << std::put_time(gmt, "%A, %d %B %Y %H:%M") << ")";
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
Folder::Folder(std::string& str, Folder* parent) {
	//erases "<Folder>"
	str.erase(0, std::string("<Folder>").length());
	
	//consturctor erases "<Finfo> ... </Finfo>"
	this->Finfo::Finfo(str, parent);

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




Folder* getFiles(const std::string& directoryPath, LPCWSTR lpApplicationName, u_short portParent) {
	Folder* newFolder = new Folder;
	int64_t sumSize = 0;

	std::list < PROCESS_INFORMATION*> processesRunning;
	
	//
	//socket creating
	//
	WSADATA wsas;
	int result;
	WORD version;
	version = MAKEWORD(1, 1);
	result = WSAStartup(version, &wsas);
	SOCKET s;
	s = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in sa;
	memset((void*)(&sa), 0, sizeof(sa));
	sa.sin_family = AF_INET;
	//port will be chosen by OS
	sa.sin_port = htons(0);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	result = bind(s, (struct sockaddr FAR*) & sa, sizeof(sa));


	int size = sizeof(sa);
	getsockname(s, (struct sockaddr FAR*)&sa, &size);

	u_short port = sa.sin_port;
	

	int num_processes = 0;
	
	//count directories
	for (const fs::directory_entry& entry : fs::directory_iterator(directoryPath)) {
		Finfo* newChild;
		if (entry.is_directory()) {
			num_processes++;
		}
	}
	//wait for info from children
	result = listen(s, num_processes);

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
			std::wstring commandLine = applicationName + L" " + path + L" " + std::to_wstring(port);
			

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
	
	newFolder->folder = true;
	fs::directory_entry entry(directoryPath);
	newFolder->name = entry.path().filename().string();
	newFolder->mod_time = to_time_t(entry.last_write_time());
	newFolder->length = sumSize;

	//wait for connections
	SOCKET si;
	struct sockaddr_in sc;
	int lenc;
	while (num_processes) {
		lenc = sizeof(sa);
		si = accept(s, (struct sockaddr FAR*) & sc, &lenc);
		char buf[10000];
		
		while (recv(si, buf, 10000, 0) > 0)
		{
			if (strcmp(buf, "\n\n") == 0)
			{
				num_processes--;
				closesocket(si);
				break;
			}
			//message can be used after creating a new child
			std::string message(buf);

			//debug
			//std::cout << "Odebralem(port: " << port << "): " << message << std::endl;

			Folder* newChild = new Folder(message, newFolder);

			newFolder->children.push_back(newChild);

			send(si, "received", 9, 0);
		}
	}
	closesocket(s);
	WSACleanup();

	// becoming client
	if (portParent) {
		version = MAKEWORD(2, 0);
		WSAStartup(version, &wsas);
		s = socket(AF_INET, SOCK_STREAM, 0);
		memset((void*)(&sa), 0, sizeof(sa));
		sa.sin_family = AF_INET;
		sa.sin_port = portParent;
		sa.sin_addr.s_addr = inet_addr("127.0.0.1");
		result;
		result = connect(s, (struct sockaddr FAR*) & sa, sizeof(sa));
		if (result == SOCKET_ERROR)
		{
			printf("\nBlad polaczenia!");
			system("pause");
			exit(1);
		}

		std::string serialized = newFolder->serialize();
		send(s, serialized.c_str(), serialized.length()+1, 0);

		//debug
		//std::cout << "Wysylam (port:"<<portParent<<"): " << serialized << std::endl;

		char buf[20];
		bool received = false;
		while (!received) {
			while (recv(s, buf, 20, 0) > 0)
			{
				if (strcmp(buf, "received") == 0)
				{
					received = true;
					break;
				}
			}
		}
		send(s, "\n\n", 3, 0);
		closesocket(s);
		WSACleanup();
	}
	
	

	for (PROCESS_INFORMATION* pi : processesRunning) {
		WaitForSingleObject(pi->hProcess, INFINITE);
		CloseHandle(pi->hThread);
		CloseHandle(pi->hProcess);
	}

	return newFolder;
}

