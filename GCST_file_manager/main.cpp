#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>
#include <Windows.h>

#define LOG_ERROR(x) std::cout << std::dec << "ERROR: "<< x << std::endl;
#define LOG_INFO(x) std::cout << std::dec <<  "INFO:  "<< x << std::endl;

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
bool fileExists(fs::path filePath) {
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


struct HEADERFILEENTRY {
	uint32_t type;
	uint32_t offset;
	uint32_t size;
	uint32_t magic;
	uint8_t name[0x20];
};

bool ALAR_unpack(fs::path fileIn, fs::path dirOut) {
	if (fileExists(fileIn)) {
		std::ifstream ifs(fileIn);
		fs::path infoFile = dirOut;
		infoFile /= "PACKAGE_INFO.txt";
		std::ofstream ofs(infoFile);

		ifs.seekg(4, SEEK_SET);
		char headerID;
		ifs.read(&headerID, 1);
		ifs.seekg(6, SEEK_SET); //skip the magical 0x61 number
		uint16_t headerFileCount;
		ifs.read(reinterpret_cast<char*>(&headerFileCount), sizeof(headerFileCount)); //stored fucking oddly. 0x83 indicates 83 files, not 131 as you would expect.
		//headerFileCount = ((headerFileCount & 0xFF00) >> 8) ^ ((headerFileCount & 0x00FF) << 8);
		LOG_INFO("Files in archive: " << std::dec << (unsigned int)headerFileCount);

		uint32_t packageID;
		uint32_t packageIDend;
		
		if (headerID == 2) {

			ifs.seekg(0, SEEK_END);
			LOG_ERROR(std::dec << ifs.tellg());

			LOG_INFO("Files in archive: " << std::hex << (unsigned int)headerFileCount);
			struct HEADERFILEENTRY* files = (struct HEADERFILEENTRY*)malloc(headerFileCount * sizeof(struct HEADERFILEENTRY));
			ifs.seekg(8, SEEK_SET);
			ifs.read(reinterpret_cast<char*>(&packageID), sizeof(packageID));
			ifs.read(reinterpret_cast<char*>(&packageIDend), sizeof(packageIDend));
			
			int64_t tempLoc = ifs.tellg();
			for (int i = 0; i < headerFileCount; i++) {
				LOG_INFO("package ID end: " << std::hex << packageIDend);
				
				
				ifs.read(reinterpret_cast<char*>(&files[i].type), 4);
				ifs.read(reinterpret_cast<char*>(&files[i].offset), 4);
				ifs.read(reinterpret_cast<char*>(&files[i].size), 4);
				ifs.read(reinterpret_cast<char*>(&files[i].magic), 4);
				LOG_INFO("file offset " << std::hex << ifs.tellg() << std::dec << " file: " << i << " type: " << std::hex << files[i].type << " offset: " << files[i].offset << " size: " << files[i].size);

				
			}
			free(files);
		}
		
		

		ifs.close();
		ofs.close();
	}
	else {
		return false;
	}
	return true;
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
			ifs.close();
			LOG_INFO(inFileTypeName);
			if ((std::string)inFileTypeName == "ALAR") {
				//do alar stuff, and put the files into a directory with the name of the original archive
				fs::path newDir = unpackedDir;
				newDir /= fileIn.filename();
				fs::create_directory(newDir);
				ALAR_unpack(fileIn.parent_path() /= fileIn.filename(), newDir);
			}
			else if ((std::string)inFileTypeName == "ALLZ") {
				//un-allz the thing, and dump the result directly into the unpacked dir
			}
			else {
				//probably already decoded or raw data, just copy them over
				fs::path newFile = unpackedDir;
				newFile /= fileIn.filename();
				if (fileExists(newFile)) {
					fs::remove(newFile);
				}
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