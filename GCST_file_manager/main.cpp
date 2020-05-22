#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>
#include <Windows.h>

#define LOG_ERROR(x) std::cout << "ERROR: "<< x << std::endl;
#define LOG_INFO(x) std::cout <<  "INFO:  "<< x << std::endl;

namespace fs = std::filesystem;

char* argvv[16];

std::string gameDir = "D:/games/Groove Coaster for Steam/";
std::string unpackedDir = "D:/games/groove_coaster_unpacked";


/* getting all things in directory:
for (auto& p : fs::directory_iterator("."))
		std::cout << p.path() << '\n';
*/

/*void startup(LPCSTR lpApplicationName)
{
	// additional information
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;

	// set the size of the structures
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	// start the program up
	CreateProcessA
	(
		lpApplicationName,   // the path
		argvv[1],                // Command line
		NULL,                   // Process handle not inheritable
		NULL,                   // Thread handle not inheritable
		FALSE,                  // Set handle inheritance to FALSE
		CREATE_NEW_CONSOLE,     // Opens file in a separate console
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi           // Pointer to PROCESS_INFORMATION structure
	);
	// Close process and thread handles. 
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}*/

bool fileExists(std::string filePath){
	std::ifstream test(filePath);
	return (test) ? true : false;
}

bool directoryExists(std::string directoryPath) {
	return (fs::is_directory(directoryPath)) ? true : false;
}

bool updateGamePath(std::string gamePath) {
	LOG_ERROR("NOT YET IMPLEMENTED");
	return false;
}



bool unpackGameData(void) {
	std::string dataDir = gameDir + "Data/";
	for (auto& p : fs::directory_iterator(dataDir)) {
		//creating a directory for the files to reside in.
		LOG_INFO("Unpacking " << p.path());
		fs::path fileIn = p.path();
		

		std::ifstream ifs(fileIn);

		if (!ifs.bad()) {
			char inFileTypeName[5] = "TEST";
			ifs.read(inFileTypeName, sizeof(char) * 4);
			LOG_INFO(inFileTypeName);
			if ((std::string)inFileTypeName == "ALAR") {
				//do alar stuff, and put the files into a directory with the name of the original archive
				fs::path newDir = unpackedDir;
				newDir /= fileIn.filename();
				fs::create_directory(newDir);
			}
			else if ((std::string)inFileTypeName == "ALLZ") {
				//un-allz the thing, and dump the result directly into the unpacked dir
			}
			else {
				//probably already decoded or raw data, just copy them over
				fs::path newFile = unpackedDir;
				newFile /= fileIn.filename();
				fs::copy_file(fileIn, newFile);
			}
		}
	}
		

	return true;
}

int main(int argc, char* argv[]) {
	argvv[1] = argv[1];

	unpackGameData();
	

	return 0;
}