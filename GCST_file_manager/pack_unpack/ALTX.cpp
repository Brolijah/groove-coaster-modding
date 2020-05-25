#include "ALTX.h"
#include <filesystem>
#include "..\common.h"
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

struct RGBA {
	char R;
	char G;
	char B;
	char A;
};

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
		if (fs::is_regular_file(fileOut)) {
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