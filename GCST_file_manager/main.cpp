#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>
#include <Windows.h>
#include <iomanip>

extern "C" {
#include "aqualead_LZSS/aqualead_LZSS.h"
}


//define logging type
#ifdef NDEBUG
#define RELEASE
#endif
#ifdef _DEBUG
#define VERBOSE
#endif

#ifdef RELEASE
#define _LOG_MANDATORY
#define _LOG_ERROR
#endif
#ifdef DEBUG
#define _LOG_MANDATORY
#define _LOG_ERROR
#define _LOG_INFO
#endif
#ifdef VERBOSE
#define _LOG_MANDATORY
#define _LOG_ERROR
#define _LOG_INFO
#define _LOG_EXTRA
#endif

#ifdef _LOG_MANDATORY
#define LOG_MANDATORY(x) std::cout << std::dec <<  "SYSTEM : " << x << std::endl;
#else
#define LOG_MANDATORY(x)
#endif
#ifdef _LOG_ERROR
#define LOG_ERROR(x) std::cout << std::dec <<      "ERROR  : " << x << std::endl;
#else
#define LOG_ERROR(x)
#endif
#ifdef _LOG_INFO
#define LOG_INFO(x) std::cout << std::dec <<       "INFO   : " << x << std::endl;
#else
#define LOG_INFO(x)
#endif
#ifdef _LOG_EXTRA
#define LOG_EXTRA(x) std::cout << std::dec <<      "VERBOSE: " << x << std::endl;
#else
#define LOG_EXTRA(x)
#endif

namespace fs = std::filesystem;

char* argvv[16];

std::string gameDir = "D:\\games\\Groove Coaster for Steam\\";
std::string unpackedDir = "D:\\games\\groove_coaster_unpacked\\";

struct RGBA {
	char R;
	char G;
	char B;
	char A;
};

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
bool directoryExists(fs::path directoryPath) {
	return (fs::is_directory(directoryPath)) ? true : false;
}

bool updateGamePath(std::string gamePath) {
	LOG_ERROR("NOT YET IMPLEMENTED");
	return false;
}


bool ALTX_unpack(fs::path fileIn, fs::path fileOut) {
	std::ifstream ifs(fileIn, std::ios::binary);
	ifs.seekg(0, std::ios::end);
	int fileSize = ifs.tellg();
	ifs.seekg(0, std::ios::beg);

	char magic[5] = "NONE";
	ifs.read(reinterpret_cast<char*>(&magic), 4);
	LOG_EXTRA("magic: " << magic);
	if ((std::string)magic == "ALTX") {
		

		while ((std::string)magic != "ALIG") { //loop until we find the ALIG
			ifs.read(reinterpret_cast<char*>(&magic), 4);
		}
		ifs.seekg(4, std::ios::cur);
		ifs.read(reinterpret_cast<char*>(&magic), 4); //read if it's palletted or not
		char magic2[5] = "NONE";
		ifs.read(reinterpret_cast<char*>(&magic2), 4); //read if it's RGBA or whatever

		uint32_t width;
		uint32_t height;
		ifs.read(reinterpret_cast<char*>(&width), 4);
		ifs.read(reinterpret_cast<char*>(&height), 4);
		LOG_EXTRA("image width: " << width << "  image height: " << height << "  palette (if any): " << magic);
		
		ifs.seekg(8, std::ios::cur); //skip two random values of which I don't know what they do yet
		struct RGBA* image = (struct RGBA*)malloc(width * height * 4);
		if ((std::string)magic == "PAL8") { //8-bit palette
			//8-bit, so 256 entries, so 1024 bytes
			struct RGBA* palette = (struct RGBA*)malloc(1024);
			if ((std::string)magic2 == "RGBA") {
				for (int i = 0; i < 256; i++) {
					ifs.read(&palette[i].R, 1);
					ifs.read(&palette[i].G, 1);
					ifs.read(&palette[i].B, 1);
					ifs.read(&palette[i].A, 1);
				}
			}
			else {
				LOG_ERROR("unknown color format");
			}
			
			//now turn the image itself into normal RGBA
			for (int i = 0; i < (width * height); i++) {
				uint8_t tmp;
				ifs.read(reinterpret_cast<char*>(&tmp), 1);
				image[i].R = palette[tmp].R;
				image[i].G = palette[tmp].G;
				image[i].B = palette[tmp].B;
				image[i].A = palette[tmp].A;
			}
			free(palette);
		}
		else if ((std::string)magic == "RGBA") {
			for (int i = 0; i < (width * height); i++) {
				ifs.read(&image[i].R, 1);
				ifs.read(&image[i].G, 1);
				ifs.read(&image[i].B, 1);
				ifs.read(&image[i].A, 1);
			}
		}
		else if ((std::string)magic == "BGRA") {
			for (int i = 0; i < (width * height); i++) {
				ifs.read(&image[i].B, 1);
				ifs.read(&image[i].G, 1);
				ifs.read(&image[i].R, 1);
				ifs.read(&image[i].A, 1);
			}
		}
		else {
			LOG_ERROR("unknown pallette format");
		}

		

		//deleting file if it exists
		LOG_EXTRA("writing to filePath " << fileOut);
		if (fileExists(fileOut)) {
			LOG_EXTRA("file already exists, so we will overwrite it");
			fs::remove(fileOut);
		}

		std::ofstream ofs(fileOut, std::ios::binary);

		//PAM-ifying file
		ofs << "P7" << '\n';
		ofs << "WIDTH " << width << '\n';
		ofs << "HEIGHT " << height << '\n';
		ofs << "DEPTH 4\n";
		ofs << "MAXVAL 255\n";
		ofs << "TUPLTYPE RGBA_ALPHA\n";
		ofs << "ENDHDR\n";

		for (int i = 0; i < (width * height); i++) {
			ofs.write(&image[i].R, 1);
			ofs.write(&image[i].G, 1);
			ofs.write(&image[i].B, 1);
			ofs.write(&image[i].A, 1);
		}
		ofs.close();
		free(image);
	}
	else {
		LOG_INFO("Supplied file is not an ALTX file");
		return false;
	}


	

	ifs.close();

	return true;
}
bool ALTX_unpack_dir(fs::path directory) {
	for (auto& p : fs::directory_iterator(directory)) {
		fs::path out = p.path().parent_path();
		std::string tmpstr = p.path().stem().string();
		tmpstr += ".pam";
		out /= tmpstr;
		try {
			ALTX_unpack(p.path(), out);
		}
		catch (fs::filesystem_error& e){
			LOG_ERROR("(in ALTX_unpack_dir) " << e.what());
			system("PAUSE");
		}
	}
	return true;
}

struct HEADERFILEENTRY {
	uint32_t type;
	uint32_t offset;
	uint32_t size;
	uint32_t magic;
	uint8_t name[0x21]; //add a last byte to act as terminator
};

bool ALAR_unpack(fs::path fileIn, fs::path dirOut) {
	if (fileExists(fileIn)) {
		std::ifstream ifs(fileIn, std::ios::binary);
		fs::path infoFile = dirOut;
		infoFile /= "PACKAGE_INFO.txt";
		std::ofstream ofs(infoFile);

		ifs.seekg(4, SEEK_SET);
		char headerID;
		ifs.read(&headerID, 1);
		ifs.seekg(6, SEEK_SET); //skip the magical 0x61 number
		uint16_t headerFileCount;
		ifs.read(reinterpret_cast<char*>(&headerFileCount), sizeof(headerFileCount)); //stored fucking oddly. 0x83 indicates 83 files, not 131 as you would expect.
		LOG_INFO("Files in archive: " << std::dec << (unsigned int)headerFileCount);

		uint32_t packageID;
		uint32_t packageIDend;
		
		if (headerID == 2) {
			
			ifs.seekg(0, std::ios::end);

			LOG_INFO("Files in archive: " << std::hex << (unsigned int)headerFileCount);
			struct HEADERFILEENTRY* files = (struct HEADERFILEENTRY*)malloc(headerFileCount * sizeof(struct HEADERFILEENTRY));
			ifs.seekg(8, std::ios::beg);
			ifs.read(reinterpret_cast<char*>(&packageID), sizeof(packageID));
			ifs.read(reinterpret_cast<char*>(&packageIDend), sizeof(packageIDend));
			
			ofs << "PACKAGEID:0x" << std::hex << packageID << "|PACKAGEIDEND:0x" << packageIDend << std::dec << std::endl;

			int64_t tempLoc = ifs.tellg();
			for (int i = 0; i < headerFileCount; i++) {
				//getting stuff from header
				ifs.read(reinterpret_cast<char*>(&files[i].type), 4);
				ifs.read(reinterpret_cast<char*>(&files[i].offset), 4);
				ifs.read(reinterpret_cast<char*>(&files[i].size), 4);
				ifs.read(reinterpret_cast<char*>(&files[i].magic), 4);
				
				//getting the file name
				//char fileName[] = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"; //the last char is 0x00 this way
				int tempPos = ifs.tellg();
				files[i].name[sizeof(files[i].name)] = '\x0';
				ifs.seekg(files[i].offset - 0x22, std::ios::beg);
				ifs.read(reinterpret_cast<char*>(files[i].name), sizeof(files[i].name)-1);
				LOG_EXTRA("current file: " << files[i].name << "  header offset: " << std::hex << ifs.tellg() << std::dec << "  file: " << i << "  type: " << std::hex << files[i].type << "  offset: " << files[i].offset << "  size: " << files[i].size);
				
				//dumping file
				ifs.seekg(files[i].offset, std::ios::beg);
				std::vector<char> fileBuffer(files[i].size);
				ifs.read(fileBuffer.data(), files[i].size);
				fs::path fileOut = dirOut;
				fileOut /= (char*)files[i].name;
				LOG_EXTRA("writing to filePath " << fileOut);
				if (fileExists(fileOut)) {
					LOG_EXTRA("file already exists, so we will overwrite it");
					fs::remove(fileOut);
				}
				std::ofstream fOut(fileOut, std::ios::binary | std::ios::ate);
				fOut.write(fileBuffer.data(), files[i].size);
				fOut.close();

				//adding metadata to PACKAGE_INFO.txt
				ofs << "TYPE:0x" << std::setw(8) << std::hex << files[i].type << std::dec << "|NAME:" << files[i].name << std::endl;


				ifs.seekg(tempPos, std::ios::beg);
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

bool ALLZ_dir(fs::path directory) {
	for (auto& p : fs::directory_iterator(directory)) {
		main_LZSS((char*)p.path().string().c_str(), (char*)p.path().string().c_str());
	}
	return true;
}

bool unpackGameData(void) {
	std::string dataDir = gameDir + "Data/";
	for (auto& p : fs::directory_iterator(dataDir)) {
		//creating a directory for the files to reside in.
		LOG_MANDATORY("Unpacking " << p.path());
		fs::path fileIn = p.path();
		

		std::ifstream ifs(fileIn, std::ios::binary);

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

				ALLZ_dir(newDir);
				//ALTX_unpack_dir(newDir);
			}
			else if ((std::string)inFileTypeName == "ALLZ") {
				//un-allz the thing, and dump the result directly into the unpacked dir
				fs::path newFile = unpackedDir;
				newFile /= fileIn.filename();
				fs::path tmp = (fileIn.parent_path() /= fileIn.filename());
				main_LZSS((char*)tmp.string().c_str(), (char*)newFile.string().c_str());


				std::ifstream ifs(newFile, std::ios::binary);
				if (!ifs.bad()) {
					char inFileTypeName2[5] = "TEST";
					ifs.read(inFileTypeName2, sizeof(char) * 4);
					ifs.close();
					LOG_INFO(inFileTypeName2);
					if ((std::string)inFileTypeName2 == "ALAR") {
						fs::path newDir = unpackedDir;

						newDir /= fileIn.filename();
						LOG_EXTRA("newDir: " << newDir);

						fs::path tmpName = unpackedDir;
						tmpName /= fileIn.filename() += ".TEMP";
						fs::rename(newDir, tmpName);

						try { fs::create_directory(newDir); }
						catch (fs::filesystem_error& e) {
							LOG_ERROR(e.what());
						}

						ALAR_unpack(tmpName, newDir);
						LOG_EXTRA("tmpName: " << tmpName);
						fs::remove(tmpName);

						ALLZ_dir(newDir);
					}
				}

				//ALTX_unpack_dir(tmp);
				

			}
			else {
				//probably already decoded or raw data, just copy them over
				fs::path newFile = unpackedDir;
				newFile /= fileIn.filename();
				if (fileExists(newFile)) {
					fs::remove(newFile);
				}
				fs::copy_file(fileIn, newFile);

				//ALTX_unpack(newFile, newFile);
			}
		}
	}
		

	return true;
}



int main(int argc, char* argv[]) {
	argvv[1] = argv[1];

	fs::path to_linklink_atx = unpackedDir;
	to_linklink_atx /= "TitleLarge.aar/LINK_LINK_FEVER_01.atx";
	fs::path to_linklink_pam = unpackedDir;
	to_linklink_pam /= "temp/LINK_LINK_FEVER_01.pam";

	fs::path to_atx = unpackedDir;
	to_atx /= "Avatar.aar/AvatarTexture.atx";
	fs::path to_pam = unpackedDir;
	to_pam /= "temp/AvatarTexture.pam";
	//ALTX_unpack(to_atx, to_pam);

	fs::path to_atx_ = unpackedDir;
	to_atx_ /= "NavigatorTexture.aar";
	ALTX_unpack_dir(to_atx_);
	//unpackGameData();
	

	return 0;
}