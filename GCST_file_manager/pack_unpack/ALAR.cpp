#include "..\common.h"
#include <filesystem> //compile with C++17 for this
#include <fstream>
#include <iostream>
#include <string>
namespace fs = std::filesystem;

struct HEADERFILEENTRY {
	uint32_t type;
	uint32_t offset;
	uint32_t size;
	uint32_t magic;
	uint8_t name[0x21]; //add a last byte to act as terminator
};


//clean up this code!
bool ALAR_unpack(fs::path fileIn, fs::path dirOut) {
	if (fs::is_regular_file(fileIn)) {
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
				ifs.read(reinterpret_cast<char*>(files[i].name), sizeof(files[i].name) - 1);
				LOG_EXTRA("current file: " << files[i].name << "  header offset: " << std::hex << ifs.tellg() << std::dec << "  file: " << i << "  type: " << std::hex << files[i].type << "  offset: " << files[i].offset << "  size: " << files[i].size);

				//dumping file
				ifs.seekg(files[i].offset, std::ios::beg);
				std::vector<char> fileBuffer(files[i].size);
				ifs.read(fileBuffer.data(), files[i].size);
				fs::path fileOut = dirOut;
				fileOut /= (char*)files[i].name;
				LOG_EXTRA("writing to filePath " << fileOut);
				if (fs::is_regular_file(fileOut)) {
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

//clean up this code!
bool ALAR_pack(fs::path dirIn, fs::path fileOut) {

	std::vector<std::string> filesToBeYeeted;

	fs::path packageInfo = dirIn;
	packageInfo /= "PACKAGE_INFO.txt";
	if (fs::is_regular_file(packageInfo)) {
		LOG_EXTRA("the package info exists for directory " << dirIn);
		/*if (packFilesWithin) {
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
		}*/

		std::ifstream ifpack(packageInfo);
		std::string tmp;
		std::vector<std::string> fileNameVector;
		std::vector<uint32_t> fileTypeVector;
		std::vector<uint32_t> fileMagicVector;
		std::vector<uint16_t> fileIdThing;
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
			std::string tmpThing;
			while ((ifpack >> tmp) && (ifpack >> tmpVal) && (ifpack >> tmp) && (ifpack >> tmpName) && (ifpack >> tmp) && (ifpack >> tmpMagic) && (ifpack)) {

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