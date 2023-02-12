#include <iostream>
#include <string>
#include "getFiles.h"


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

int main() {
	const std::string path = R"(/home/szymon/Documents)";
	Folder* root;

	root = getFiles(path);
	writeFiles(root);

	delete root;
	return 0;
}
