#include "general.h"
#include "../common.h"
#include <filesystem>
#include <fstream>
#include "../aqualead_types.h"

#include "ALTX.h"
#include "ALLZ.h"
#include "ALAR.h"
#include "ALSN.h"
#include "ALTB.h"

namespace fs = std::filesystem;

bool pack(fs::path directoryIn, fs::path fileOut) {
	LOG_EXTRA("packing...");
	to_game_files(directoryIn);
	ALAR_pack(directoryIn, fileOut);
	return true;
}

bool unpack(fs::path fileIn, fs::path directoryOut) {
	LOG_EXTRA("unpacking...");
	ALLZ_decompress(fileIn.string().c_str(), fileIn.string().c_str());
	ALAR_unpack(fileIn, directoryOut);
	to_human_files(directoryOut);
	return true;
}

bool to_game_files(fs::path directoryIn) {
	for (auto& p : fs::directory_iterator(directoryIn)) {
		LOG_EXTRA("current file: " << p.path().filename());
		if (p.path().extension() == ".ogg") {
			fs::path outFile = p.path().parent_path();
			outFile /= (p.path().stem() += ".asn");
			ALSN_pack(p.path(), outFile);
		}
	}

	return true;
}

bool to_human_files(fs::path directoryIn) {
	for (auto& p : fs::directory_iterator(directoryIn)) {
		LOG_EXTRA("current file: " << p.path().filename());
		ALLZ_decompress(p.path().string().c_str(), p.path().string().c_str());
		std::ifstream ifs(p.path());
		if (!ifs.bad()) {
			uint32_t magic = 0;
			ifs.read(reinterpret_cast<char*>(&magic), 4);
			ifs.close();
			
			LOG_EXTRA("magic: " << std::hex << magic);
			switch (magic) {
			case AL_TYPE_ALTX: {
				LOG_EXTRA("  ALTX file");
				fs::path fOut = p.path().parent_path();
				fOut /= (p.path().stem() += ".pam");
				ALTX_unpack_pam(p.path(), fOut);
				break;
			};
			case AL_TYPE_ALSN: {
				LOG_EXTRA("  ALSN file");
				fs::path fOut = p.path().parent_path();
				fOut /= (p.path().stem() += ".ogg");
				ALSN_unpack(p.path(), fOut);
				break;
			};
			case AL_TYPE_ALTB: {
				LOG_EXTRA("  ALTB file");
				fs::path fOut = p.path().parent_path();
				fOut /= (p.path().stem() += ".txt");
				ALTB_unpack(p.path(), fOut);
				break;
			}
			default:
				LOG_INFO("file " << p.path().filename() << "is not a file that can be humanized (yet?)");
				break;
			};

				
		}
	}

	return true;
}

bool unpack_game(fs::path gameDir, fs::path unpackedDir) {
	if (fs::is_directory(gameDir) && fs::is_directory(unpackedDir)) {
		for (auto& p : fs::directory_iterator(gameDir)) {
			fs::path newDir = unpackedDir;
			newDir /= p.path().filename();
			if (fs::is_directory(newDir)) {
				try {
					fs::remove_all(newDir);
				}
				catch (fs::filesystem_error& e) {
					LOG_ERROR("in unpack_game: " << e.what());
				}
				
			}
			fs::create_directory(newDir);
			LOG_MANDATORY("unpacking: " << newDir);
			unpack(p.path(), newDir);
		}
	}
	else {
		LOG_WARN("either your game or unpack dir are invalid");
	}
	
	return true;
}