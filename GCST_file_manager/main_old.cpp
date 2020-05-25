//#define NOPE
#ifdef NOPE

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
std::string repackedDir = "D:\\games\\groove Coaster for Steam - Repacked\\Data\\";


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
	//temp
	return false;
}

template<typename T>
void pop_front(std::vector<T>& v)
{
	if (v.size() > 0) {
		v.erase(v.begin());
	}
}

bool ALTX_unpack(fs::path fileIn, fs::path fileOut) {
	std::ifstream ifs(fileIn, std::ios::binary);
	ifs.seekg(0, std::ios::end);
	std::streamoff fileSize = ifs.tellg();
	ifs.seekg(0, std::ios::beg);

	char magic[5] = "NONE";
	ifs.read(reinterpret_cast<char*>(&magic), 4);
	LOG_EXTRA("magic: " << magic);
	if ((std::string)magic == "ALTX") {
		

		fs::path infoFilePath = fileIn.parent_path();
		infoFilePath /= fileIn.stem();
		infoFilePath += ".HEADER";
		LOG_EXTRA("infofilepath: " << infoFilePath);
		std::ofstream infoFile(infoFilePath, std::ios::binary);
		infoFile.seekp(infoFile.beg);
		infoFile << fileIn.filename().string();
		infoFile << " ";
		std::streamoff tmpLoc = ifs.tellg();
		ifs.seekg(ifs.beg);
		char tmpMagic[5] = "NONE";
		while ((std::string)tmpMagic != "ALIG") {
			ifs.read(reinterpret_cast<char*>(&tmpMagic), 4);
			infoFile.write(reinterpret_cast<char*>(&tmpMagic), 4);
			//LOG_EXTRA("write!");
		}
		infoFile.close();
		ifs.seekg(tmpLoc);


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
		struct RGBA* image = (struct RGBA*)malloc((uint64_t)width * (uint64_t)height * 4);
		if ((std::string)magic == "PAL8") { //8-bit palette
			//8-bit, so 256 entries, so 1024 bytes
			struct RGBA* palette = (struct RGBA*)malloc(1024);
			if ((std::string)magic2 == "RGBA") {
				for (int i = 0; i < 256; i++) {
					ifs.read(&palette[i].R, 4); //should do the same as the following four lines, but faster
					//ifs.read(&palette[i].R, 1);
					//ifs.read(&palette[i].G, 1);
					//ifs.read(&palette[i].B, 1);
					//ifs.read(&palette[i].A, 1);
				}
			}
			else {
				LOG_ERROR("unknown color format");
			}
			
			//now turn the image itself into normal RGBA
			for (unsigned int i = 0; i < (width * height); i++) {
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
			for (unsigned int i = 0; i < (width * height); i++) {
				ifs.read(&image[i].R, 4); //should do the same as the following four lines, but faster
				//ifs.read(&image[i].R, 1);
				//ifs.read(&image[i].G, 1);
				//ifs.read(&image[i].B, 1);
				//ifs.read(&image[i].A, 1);
			}
		}
		else if ((std::string)magic == "BGRA") {
			for (unsigned int i = 0; i < (width * height); i++) {
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

		for (unsigned int i = 0; i < (width * height); i++) {
			ofs.write(&image[i].R, 4); //should do the same as the following four lines, but faster
			//ofs.write(&image[i].R, 1);
			//ofs.write(&image[i].G, 1);
			//ofs.write(&image[i].B, 1);
			//ofs.write(&image[i].A, 1);
		}
		ofs.close();
		free(image);
		ifs.close();
		fs::remove(fileIn);
	}
	else {
		LOG_INFO("Supplied file is not an ALTX file");
		return false;
	}


	

	ifs.close();

	return true;
}
bool ALTX_unpack_dir(fs::path directory) {
	try {
		//fs::path imageHeaders = directory;
		//imageHeaders /= "IMAGE_INFO.txt";
		//if (fs::is_regular_file(imageHeaders)) {
		//	LOG_EXTRA("removing file " << imageHeaders << " because we're unpacking the entire directory");
		//	fs::remove(imageHeaders);
		//}
		for (auto& p : fs::directory_iterator(directory)) {

			if (fs::is_regular_file(p.path())) {
				fs::path out = p.path().parent_path();
				std::string tmpstr = p.path().stem().string();
				tmpstr += ".pam";
				out /= tmpstr;
				try {
					ALTX_unpack(p.path(), out);
				}
				catch (fs::filesystem_error& e) {
					LOG_ERROR("(in ALTX_unpack_dir) " << e.what());
					return false;
				}
			}

		}
	}
	catch (fs::filesystem_error& e) {
		LOG_ERROR(e.what());
		return false;
	}
	
	return true;
}
bool ALTX_repack_pam(fs::path fileIn, fs::path fileOut) {
	std::ifstream ifs(fileIn, std::ios::binary);
	std::string tmp;
	ifs >> tmp;

	uint32_t height = 0;
	uint32_t width = 0;
	uint32_t depth = 0;
	std::string sType = "";
	uint32_t maxval = 0;

	//fix this to accept any order, since the file specification allows for that
	if (tmp == "P7") {
		while (tmp != "ENDHDR") {
			ifs >> tmp;
			if (tmp == "WIDTH")
				ifs >> width;
			if (tmp == "HEIGHT")
				ifs >> height;
			if (tmp == "DEPTH")
				ifs >> depth;
			if (tmp == "MAXVAL")
				ifs >> maxval;
			if (tmp == "TUPLTYPE")
				ifs >> sType;
		}
		LOG_INFO("repacking file " << fileIn.filename() << ", width=" << width << " height=" << height << " depth=" << depth << " type=" << sType << " maxval=" << maxval);

		if (fs::is_regular_file(fileOut)) {
			fs::remove(fileOut);
		}
		std::ofstream ofs(fileOut, std::ios::binary);
		ofs << "ALTX";

		ofs.close();
	}
	else {
		LOG_ERROR("input file is not a pam file!")
	}
	ifs.close();
	return true;
}
bool ALTX_repack_tga(fs::path fileIn, fs::path fileOut) {
	LOG_ERROR("not imeplemented");
	throw("not implemented");
	//http://paulbourke.net/dataformats/tga/
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
			
			ofs << "PACKAGEID: 0x" << std::hex << packageID << " PACKAGEIDEND: 0x" << packageIDend << std::dec << std::endl;

			int64_t tempLoc = ifs.tellg();
			for (int i = 0; i < headerFileCount; i++) {
				//getting stuff from header
				ifs.read(reinterpret_cast<char*>(&files[i].type), 4);
				ifs.read(reinterpret_cast<char*>(&files[i].offset), 4);
				ifs.read(reinterpret_cast<char*>(&files[i].size), 4);
				ifs.read(reinterpret_cast<char*>(&files[i].magic), 4);
				
				//getting the file name
				//char fileName[] = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"; //the last char is 0x00 this way
				std::streamoff tempPos = ifs.tellg();
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
				ofs << "TYPE: 0x" << std::setw(8) << std::setfill('0') << std::hex << files[i].type << std::dec << " NAME: " << files[i].name << " MAGIC: 0x" << std::hex << std::setw(8) << files[i].magic << std::endl;


				ifs.seekg(tempPos, std::ios::beg);
			}
			free(files);
		}
		
		

		ifs.close();
		ofs.close();
	}
	else {
		LOG_ERROR("file " << fileIn << " doesn't exist!");
		return false;
	}
	return true;
}
bool ALAR_repack(fs::path dirIn, fs::path fileOut, bool packFilesWithin) {

	std::vector<std::string> filesToBeYeeted;

	fs::path packageInfo = dirIn;
	packageInfo /= "PACKAGE_INFO.txt";
	if (fs::is_regular_file(packageInfo)) {
		LOG_EXTRA("the package info exists for directory " << dirIn);
		if (packFilesWithin) {
			for (auto& p : fs::directory_iterator(dirIn)) {
				LOG_EXTRA("   " << p.path());
				//LOG_EXTRA("packed file name: " << packedFile);

				if ((p.path().filename().string() != "PACKAGE_INFO.txt") && (p.path().filename().c_str()[0] != '.')) {
					if (p.path().extension().string() == ".pam") {
						//ALTX_repack_pam(p.path(), packedFile);
						//LOG_EXTRA(".pam file!");
					}
					if (p.path().extension().string() == ".ogg") {
						//ALSN_repack
					}
					//filesToBeYeeted.push_back(packedFile.string()); //add the temporary packed file to the cleanup list
					//add more when more filetypes are reverse-engineered
				}
			}
		}

		std::ifstream ifpack(packageInfo);
		std::string tmp;
		std::vector<std::string> fileNameVector;
		std::vector<uint32_t> fileTypeVector;
		std::vector<uint32_t> fileMagicVector;
		ifpack >> tmp;
		uint32_t packageID = 0;
		uint32_t packageIDend = 0;
		if (tmp == "PACKAGEID:") {
			
			ifpack >> tmp;
			sscanf(tmp.c_str(), "0x%x", &packageID);
			
			ifpack >> tmp;
			ifpack >> tmp;
			sscanf(tmp.c_str(), "0x%x", &packageIDend);
			LOG_INFO("packageID: 0x" << std::hex << packageID << "  packageIDend: 0x" << packageIDend);

			std::string tmpVal;
			std::string tmpName;
			std::string tmpMagic;
			while ((ifpack >> tmp) && (ifpack >> tmpVal) && (ifpack >> tmp) && (ifpack >> tmpName) && (ifpack >> tmp) && (ifpack >> tmpMagic)) {
				
				uint32_t packageType;
				uint32_t packageMagic;
				sscanf(tmpVal.c_str(), "0x%x", &packageType);
				sscanf(tmpMagic.c_str(), "0x%x", &packageMagic);
				LOG_EXTRA("packageType: 0x" << std::hex << packageType << "  name: " << tmpName << "  magic: " << packageMagic);
				fileNameVector.push_back(tmpName);
				fileTypeVector.push_back(packageType);
				fileMagicVector.push_back(packageMagic);
				//packageIDend = packageType;
			}
		}
		ifpack.close();

		//move on to constructing the new package
		if (fs::is_regular_file(fileOut)) {
			fs::remove(fileOut);
		}
		std::ofstream ofs(fileOut, std::ios::binary);
		if (!ofs.bad()) {
			ofs << "ALAR\x2";
			ofs << "\x61"; //no clue what this value does, but we'll see

			uint16_t fileCount = fileNameVector.size();
			LOG_EXTRA("filecount: " << fileCount);
			ofs.write(reinterpret_cast<char*>(&fileCount), 2);
			ofs.write(reinterpret_cast<char*>(&packageID), 4);
			ofs.write(reinterpret_cast<char*>(&packageIDend), 4);
			for (int i = 0; i < fileCount; i++) {
				ofs << "emptyindexentry!";
			}
			ofs.write("\x1\x1", 2); //OG file also has this, dunno why
			for (int i = 0; i < fileCount; i++) {
				if (fileNameVector.front().size() > 0x22) {
					LOG_ERROR("filename " << fileNameVector.front() << " is longer than what .aar packages can handle (max 34 characters)");
					return false;
				}
				ofs << fileNameVector.front();
				ofs.seekp((0x22 - fileNameVector.front().size()), std::ios::cur);
				std::streamoff fileOffsetLoc = ofs.tellp();
				fs::path dataFileToPack = dirIn;
				dataFileToPack /= fileNameVector.front();

				if (fs::is_regular_file(dataFileToPack)) {
					LOG_EXTRA("file " << dataFileToPack << "exists, and we'll pack it");
					std::ifstream ifs(dataFileToPack, std::ios::binary | std::ios::ate);
					if (!ifs.bad()) {
						uint32_t fileSize = ifs.tellg();
						ifs.seekg(ifs.beg);
						std::vector<char> fileBuffer(fileSize);
						LOG_EXTRA("filesize: " << fileSize);
						ifs.read(fileBuffer.data(), fileSize);
						ofs.write(fileBuffer.data(), fileSize);

						//update header
						std::streamoff tmpLoc = ofs.tellp();
						ofs.seekp((16 + (i * 16)), ofs.beg);
						ofs.write(reinterpret_cast<char*>(&fileTypeVector.front()), 4);
						uint32_t fileOffset = (uint32_t)fileOffsetLoc;
						LOG_EXTRA("fileOffset: " << fileOffset);
						ofs.write(reinterpret_cast<char*>(&fileOffset), 4);
						ofs.write(reinterpret_cast<char*>(&fileSize), 4);
						//ofs.write("loli", 4); //a constant of which I have no clue what it does
						ofs.write(reinterpret_cast<char*>(&fileMagicVector.front()), 4);
						ofs.seekp(tmpLoc, ofs.beg);
					}
				}
				

				//ofs << "FILE HERE";
				pop_front(fileTypeVector);
				pop_front(fileNameVector);
				pop_front(fileMagicVector);
			}

		}
		ofs.close();

	}
	else {
		LOG_EXTRA("the package info doesn't exist for directory " << dirIn);
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
				try {
					fs::create_directory(newDir);
				}
				catch (fs::filesystem_error& e) {
					LOG_ERROR(e.what());
				}
				ALAR_unpack(fileIn.parent_path() /= fileIn.filename(), newDir);

				ALLZ_dir(newDir);
				ALTX_unpack_dir(newDir);
			}
			else if ((std::string)inFileTypeName == "ALLZ") {
				//un-allz the thing, and dump the result directly into the unpacked dir
				fs::path newFile = unpackedDir;
				newFile /= fileIn.filename();
				fs::path tmp = (fileIn.parent_path() /= fileIn.filename());
				if (fs::is_directory(newFile)) {
					fs::remove_all(newFile);
				}
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

				ALTX_unpack_dir(newFile);
				

			}
			else {
				//probably already decoded or raw data, just copy them over
				fs::path newFile = unpackedDir;
				newFile /= fileIn.filename();
				if (fileExists(newFile)) {
					fs::remove(newFile);
				}
				fs::copy_file(fileIn, newFile);

				ALTX_unpack(newFile, newFile);
			}
		}
	}
		

	return true;
}
bool repackGameData(void) {
	std::string dataOutDir = repackedDir;
	std::string dataInDir = unpackedDir;
	if (!fs::is_directory(dataOutDir) || !fs::is_directory(dataInDir)) {
		LOG_ERROR("one of the supplied directories for repacking is wrong/inaccessible");
		return false;
	}

	for (auto& p : fs::directory_iterator(dataInDir)) {
		//still gotta check what files need to actually be packed, so repacking won't take 5 years

		//check if the file begins with a dot or if it is a package info file, because we want to ignore those
		if ((p.path().filename().c_str()[0] != '.') && (p.path().filename().string() != "PACKAGE_INFO.txt")) {
			LOG_EXTRA("the entry " << p.path().filename() << " is not an ignored file");

			if (fs::is_directory(p.path())) {
				LOG_EXTRA("the entry " << p.path().filename() << " is a directory");
				fs::path outFile = repackedDir;
				outFile /= p.path().filename();
				ALAR_repack(p.path(), outFile, true);
			}
			else if (fs::is_regular_file(p.path())) {
				LOG_EXTRA("the entry " << p.path().filename() << " is a file");
			}
			else {
				LOG_ERROR("this entry somehow doesn't exist");
			}
		}
		else {
			LOG_EXTRA("the entry " << p.path().filename() << " is an ignored file");
		}
		
	}



}


int main(int argc, char* argv[]) {
	argvv[1] = argv[1];

	fs::path to_linklink_atx = unpackedDir;
	to_linklink_atx /= "TitleLarge.aar/LINK_LINK_FEVER_01.atx";
	fs::path to_linklink_pam = unpackedDir;
	to_linklink_pam /= "temp/LINK_LINK_FEVER_01.pam";

	fs::path to_atx = unpackedDir;
	to_atx /= "NavigatorTexture.aar/";
	fs::path to_pam = unpackedDir;
	to_pam /= "NavigatorTexture/NavigatorTexture.pam";
	//ALTX_unpack(to_atx, to_pam);


	//ALTX_unpack_dir(to_atx);
	unpackGameData();
	fs::path inAlar = unpackedDir;
	inAlar /= "Stage00951.aar";
	fs::path outAlar = unpackedDir;
	outAlar /= "out.aar";
	//ALAR_repack(inAlar, outAlar, false);
	//main_LZSS((char*)"D:\\games\\Groove Coaster for Steam\\Data\\NavigatorTexture.aar", (char*)"D:\\games\\groove_coaster_unpacked\\NavigatorTexture.aar\\NavigatorTexture.aar");
	//repackGameData();
	fs::path tttt = unpackedDir;
	tttt /= "NavigatorTexture.aar/NavigatorTexture.pam";
	fs::path ttttt = unpackedDir;
	ttttt /= "NavigatorTexture.aar/out_NavigatorTexture.atx";
	//ALTX_unpack(ttttt, tttt);

	fs::path tutaar = gameDir;
	tutaar /= "Data";
	tutaar /= "Stage00382.aar";
	fs::path tutunp = "D:\\games\\tmp";
	fs::path tutrep = "D:\\games\\Stage00382.aar";

	fs::path artaar = gameDir;
	artaar /= "Data";
	artaar /= "StageTextureJp.aar";
	fs::path artunp = "D:\\games\\tmp3";

	//ALAR_unpack(tutaar, tutunp);
	//ALAR_unpack(artaar, artunp);
	//ALTX_unpack("D:\\games\\out_Shiva_01.raw", "D:\\games\\out_Shiva_human.pam");
	//ALAR_repack(artunp, artaar, false);
	//ALAR_repack(tutunp, tutrep, false);

	return 0;
}

#endif