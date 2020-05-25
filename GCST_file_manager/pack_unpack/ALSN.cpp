#include "ALSN.h"
#include "../common.h"
#include <fstream>
#include "../aqualead_types.h"
#include <vector>

namespace fs = std::filesystem;

bool ALSN_unpack(std::filesystem::path fileIn, std::filesystem::path fileOut) {
	//take off the 32-byte header and dump it in a separate file
	std::ifstream ifs(fileIn, std::ios::binary);
	if (!ifs.bad()) {
		uint32_t magic = 0;
		ifs.read(reinterpret_cast<char*>(&magic), 4);
		if (magic == AL_TYPE_ALSN) {
			ifs.seekg(0, std::ios::end);
			LOG_EXTRA("size of infile: " << ifs.tellg());
			std::streamoff fileSize = ifs.tellg();
			std::vector<char> headerBuffer(32);
			std::vector<char> soundBuffer(fileSize - 32);
			ifs.seekg(0, std::ios::beg);
			ifs.read(reinterpret_cast<char*>(headerBuffer.data()), 32);
			ifs.read(reinterpret_cast<char*>(soundBuffer.data()), fileSize - 32);
			ifs.close();
			fs::remove(fileIn);

			std::ofstream ofsData(fileOut, std::ios::binary);
			fs::path headerpath = fileOut.parent_path();
			headerpath /= (fileOut.stem() += ".HEADER");
			std::ofstream ofsHeader(headerpath, std::ios::binary);
			if (!ofsData.bad() && !ofsHeader.bad()) {
				ofsHeader.write(reinterpret_cast<char*>(headerBuffer.data()), 32);
				ofsData.write(reinterpret_cast<char*>(soundBuffer.data()), fileSize - 32);
				LOG_EXTRA("the ALSN file has been split");
			}
			ofsData.close();
			ofsHeader.close();
		}
		ifs.close();
	}
	return true;
}
bool ALSN_pack(std::filesystem::path fileIn, std::filesystem::path fileOut) {
	LOG_ERROR("NOT IMPLEMENTED");
	return true;
}