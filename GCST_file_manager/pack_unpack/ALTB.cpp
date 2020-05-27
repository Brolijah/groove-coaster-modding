#include "ALTB.h"
#include <filesystem>
#include "../common.h"
#include <fstream>
#include <sstream>
#include "../aqualead_types.h"

namespace fs = std::filesystem;


bool ALTB_unpack(fs::path fileIn, fs::path fileOut) {

	std::ifstream ifs(fileIn, std::ios::binary);
	if (!ifs.bad()) {
		uint32_t magic = 0;
		ifs.read(reinterpret_cast<char*>(&magic), 4);
		if (magic == AL_TYPE_ALTB) {
			LOG_EXTRA("magic inside: " << std::hex << magic);

			ifs.seekg(6); //this is where the amount of id-entries total is noted
			uint16_t idEntries;
			ifs.read(reinterpret_cast<char*>(&idEntries), 2);
			LOG_EXTRA("idEntries: " << idEntries);

			//at pos 0x14, there's an uint16 with the offset to the start of text
			uint16_t textOffset = 0;
			ifs.seekg(0x14, std::ios::beg);
			ifs.read(reinterpret_cast<char*>(&textOffset), 2);
			LOG_EXTRA("textoffset: " << textOffset);

			//at pos 10, there's an uint16 with offset to the start of id's
			uint16_t idOffset = 0;
			ifs.seekg(10, std::ios::beg);
			ifs.read(reinterpret_cast<char*>(&idOffset), 2);
			LOG_EXTRA("idOffset: " << idOffset);

			while (magic != AL_TYPE_ALRD) {
				ifs.read(reinterpret_cast<char*>(&magic), 4);
				//LOG_EXTRA("magic inside: " << std::hex << magic);
			}
			std::streamoff alrdPos = ifs.tellg();
			LOG_EXTRA("found ALRD at position " << ifs.tellg());
			//after ALRD, there's 16 bytes of header, and then the idNames will start
			ifs.seekg(2, std::ios::cur);
			uint8_t idCount = 0;
			ifs.read(reinterpret_cast<char*>(&idCount), 1);

			ifs.seekg(alrdPos + 16, std::ios::beg); //move to the start of the idName
			//std::string do this later, I have no clue yet how to read the names properly

			ifs.seekg(idOffset, std::ios::beg); //move to start of id's
			std::ofstream ofs(fileOut, std::ios::binary);
			/*if (!ofs.bad()) { //reading the id's into a file
				for (int a = 0; a < idEntries; a++) {
					ofs << "entry " << std::setw(3) << std::setfill('0')  << std::dec << a << " ";
					for (int i = 0; i < idCount; i++) {
						uint32_t tmp = 0;
						ifs.read(reinterpret_cast<char*>(&tmp), 4);
						//LOG_EXTRA("ID: " << tmp);
						ofs << "ID: 0x" << std::hex << std::setw(8) << std::setfill('0') << tmp << " ";
					}
					ofs << "\n";
				}
			}*/
			std::streamoff tmpOff = ifs.tellg();
			ifs.seekg((uint64_t)textOffset + 1, std::ios::beg);
			if (!ofs.bad()) {
				std::string tmp = "  ";
				int a = 0;
				while (tmp != "\x00") {
					std::getline(ifs, tmp, '\x00');
					LOG_EXTRA("curr=" << a << " with text: " << tmp);
					ofs << "curr=" << std::dec << a << "with text: " << tmp << "\n";
					a++;
				}
				for (int a = 0; a < (idEntries * 5); a++) {
					
					
				}
			}
			ofs.close();
		}
	}
	else {
		LOG_ERROR("bad");
	}

	return true;
}