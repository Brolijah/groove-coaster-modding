#include "ALTB.h"
#include <filesystem>
#include "../common.h"
#include <fstream>
#include "../aqualead_types.h"

namespace fs = std::filesystem;


bool ALTB_unpack(fs::path fileIn, fs::path fileOut) {

	std::ifstream ifs(fileIn, std::ios::binary);
	if (!ifs.bad()) {
		uint32_t magic = 0;
		ifs.read(reinterpret_cast<char*>(&magic), 4);
		if (magic == AL_TYPE_ALTB) {
			LOG_EXTRA("magic inside: " << std::hex << magic);
			while (magic != AL_TYPE_ALRD) {
				ifs.read(reinterpret_cast<char*>(&magic), 4);
				//LOG_EXTRA("magic inside: " << std::hex << magic);
			}
			LOG_EXTRA("found ALRD at position " << ifs.tellg());
			ifs.seekg(2, std::ios::cur);
			uint8_t 
		}
	}
	else {
		LOG_ERROR("bad");
	}

	return true;
}