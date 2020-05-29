#include "ALTB.h"
#include <filesystem>
#include "../common.h"
#include <fstream>
#include <sstream>
#include "../aqualead_types.h"

namespace fs = std::filesystem;

//ofs << "int;string;string     ;string;int   ;string;int    ;int     ;int       ;int       ;int     ;int      ;string;int         ;int         ;int       ;int        ;int" << "\n";
//uint8_t intOrString[64] = { 0,  1,     1,          1,     0,     1,     0,      0,       0,         0,         0,       0,        1,     0,           0,           0,         0,          0 };
//ofs << "ID ;Title ;!identifier;Artist;Source;Yomi  ;GenreID;Playtime;diffSimple;diffNormal;diffHard;diffExtra;BPM   ;bgmvolSimple;bgmVolNormal;bgmvolHard;bgmVolExtra;shotVolSimple;shotVolNormal;shotVolHard;shotVolExtra;JudgeDelay" << "\n";

const std::string tableTune[][2]{
	{"ID", "int"},
	{"Title", "string"},
	{"identifier", "string"},
	{"Artist", "string"},
	{"Source", "string"}, //not 100% sure
	{"Yomi", "string"},
	{"GenreID", "int"},
	{"Playtime", "int"},
	{"diffSimple", "int"},
	{"diffNormal", "int"},
	{"diffHard", "int"},
	{"diffExtra", "int"},
	{"BPM", "string"},
	{"bgmVolSimple", "int"},
	{"bgmVolNormal", "int"},
	{"bgmVolHard", "int"},
	{"bgmVolExtra", "int"},
	{"shotVolSimple", "int"},
	{"shotVolNormal", "int"},
	{"shotVolHard", "int"},
	{"shotVolExtra", "int"},
	{"judgeDelay", "int"},
	{"selectSeekStartTime", "int"},
	{"selectSeekEndTime", "int"},
	{"defaultSkinID", "int"},
	{"spFlag", "int"},
	{"bgmFileNameBase", "string"},
	{"bgmFileNameBaseSimple", "string"},
	{"bgmFileNameBaseNormal", "string"},
	{"bgmFileNameBaseHard", "string"},
	{"bgmFileNameBaseExtra", "string"},
	{"shotFileNameSimple", "string"},
	{"shotFileNameNormal", "string"},
	{"shotFileNameHard", "string"},
	{"shotFileNameExtra", "string"},
	{"stageFileNameSimple", "string"},
	{"stageFileNameNormal", "string"},
	{"stageFileNameHard", "string"},
	{"stageFileNameExtra", "string"},
	{"tagFileNameAddEn", "string"}, //i think?
	{"musicpanelcount", "int"},
	{"lockTypeOffline", "int"},
	{"Explanation", "int"},
	{"Recommended", "int"}, //"recommended for beginners"
	{"TitleEn", "string"},
	{"artistEn", "string"},
	{"GCprice", "int"},
	{"DLCappID", "int"}, //???
	{"bgmFileIDSimple", "int"},
	{"bgmFileIDNormal", "int"},
	{"bgmFileIDHard", "int"},
	{"bgmFileIDExtra", "int"},
	{"shotFileIDSimple", "int"},
	{"shotFileIDNormal", "int"},
	{"shotFileIDHard", "int"},
	{"shotFileIDExtra", "int"},
	{"lastPlayDateTime", "int"},
	{"played", "int"},
	{"favorite", "int"},
	{"unlock", "int"},
	{"enable", "int"},
	{"difficultySortValue", "int"},
	//and more, so something is off... there's only supposed to be 64 entries
	{"", ""},
	{"", ""},
	{"", ""},
	{"", ""},
	{"", ""},
	{"", ""},
	{"", ""},
	{"", ""},
	{"", ""},
	{"", ""},
	{"", ""},
	{"", ""},
	{"", ""},
	{"", ""},
	{"", ""},
	{"", ""},
	{"", ""},
	{"", ""},
};

bool ALTB_unpack(fs::path fileIn, fs::path fileOut) {
	

	if (fileIn.filename() != "TuneStage.atb") {
		LOG_ERROR("ALTB_unpack ONLY WORKS FOR TuneStage.atb! (for now)");
		return false;
	}
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

			//at pos 20, there's an uint16 with the offset to the start of text
			uint16_t textOffset = 0;
			ifs.seekg(20, std::ios::beg);
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

			fs::path headerPath = fileOut.parent_path();
			headerPath /= (fileOut.stem() += ".HEADER");
			std::ofstream ofh(headerPath, std::ios::binary);
			ifs.seekg(0, std::ios::beg);
			std::vector<char> fileBuffer(idOffset);
			ifs.read(fileBuffer.data(), idOffset);
			ofh.write(fileBuffer.data(), idOffset);
			ofh.close();

			std::ofstream ofs(fileOut, std::ios::binary);
			ifs.seekg(idOffset, std::ios::beg); //move to start of id's
			if (fileIn.filename() == "TuneStage.atb") {
				LOG_INFO("tunestage found! dumping into csv");
				if (!ofs.bad()) {
					for (int i = 0; i < 64; i++) {
						ofs << tableTune[i][1] << ";";
					}
					ofs << "\n";
					for (int i = 0; i < 64; i++) {
						ofs << tableTune[i][0] << ";";
					}
					ofs << "\n";

					for (int a = 0; a < idEntries; a++) {
						for (int i = 0; i < 64; i++) {

							if (tableTune[i][1] == "string") {
								uint32_t tmp = 0;
								ifs.read(reinterpret_cast<char*>(&tmp), 4);
								std::streamoff tmpOff = ifs.tellg();
								ifs.seekg((uint64_t)textOffset + tmp, std::ios::beg);
								std::string tmpStr;
								std::getline(ifs, tmpStr, '\x0');
								ifs.seekg(tmpOff, std::ios::beg);
								ofs << tmpStr << ";";
							}
							else {
								uint32_t tmp = 0;
								ifs.read(reinterpret_cast<char*>(&tmp), 4);
								ofs << tmp << ";";
							}
							
						}
						ofs << "\n";
					}
				}
			}

			/*std::streamoff tmpOff = ifs.tellg();
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
			}*/
			ofs.close();
		}
	}
	else {
		LOG_ERROR("bad");
	}

	return true;
}

bool ALTB_pack(fs::path fileIn, fs::path fileOut) {
	
	

	fs::path headerPath = fileIn.parent_path();
	headerPath /= (fileIn.stem() += ".HEADER");
	if (!fs::is_regular_file(headerPath)) {
		LOG_ERROR("header for file " << fileIn.filename() << " doesn't exist!");
		return false;
	}
	std::ifstream ifh(headerPath, std::ios::binary);

	std::ifstream ifs(fileIn, std::ios::binary);
	std::ofstream ofs(fileOut, std::ios::binary);
	std::vector<std::string> textVector;
	std::vector<std::streamoff> offsetVector;

	uint32_t textOffset = 696969;
	uint32_t indexOffset = 256;

	if (!ifs.bad() && !ofs.bad() && !ifh.bad()) {

		ifh.seekg(0, std::ios::end);
		uint64_t headerSize = (uint64_t)ifh.tellg();
		LOG_EXTRA("sizeof buffer:" << headerSize);
		std::vector<char> headerBuffer(headerSize);
		ifh.seekg(0, std::ios::beg);
		ifh.read(headerBuffer.data(), headerSize);
		ifh.seekg(6);
		uint32_t indexColumns;
		ifh.read(reinterpret_cast<char*>(&indexColumns), 4);
		ifh.close();

		ofs.seekp(0, std::ios::beg);
		ofs.write(headerBuffer.data(), headerSize);
		indexOffset = (uint32_t)ofs.tellp();



		std::string tmp;
		std::getline(ifs, tmp, '\n');
		std::stringstream ss(tmp);
		std::vector<bool> intOrStr(indexColumns);
		int tmpCounter = 0;
		while (std::getline(ss, tmp, ';')) {
			LOG_EXTRA("linepart: " << tmp);
			if (tmp == "string") {
				intOrStr[tmpCounter] = 1;
			}
			else {
				intOrStr[tmpCounter] = 0;
			}
			tmpCounter++;
		}

		std::getline(ifs, tmp, '\n');

		std::string tmpLine;
		uint16_t indexEntryCount = 0;
		ofs.seekp(indexOffset, std::ios::beg);
		while (std::getline(ifs, tmpLine)) {   // get a whole line
			std::stringstream ss(tmpLine);
			tmpCounter = 0;
			while (std::getline(ss, tmpLine, ';')) {
				if (intOrStr[tmpCounter] == 1 && tmpLine != "") { //string
					offsetVector.push_back(ofs.tellp());
					textVector.push_back(tmpLine);
					ofs.seekp(4, std::ios::cur);
				}
				else if (tmpLine == "") {
					ofs.write("\x0\x0\x0\x0", 4);
				}
				else {
					uint32_t tmpVal;
					if (sscanf(tmpLine.c_str(), "%d", &tmpVal)) {
						ofs.write(reinterpret_cast<char*>(&tmpVal), 4);
					}
				}
				tmpCounter++;
			}
			indexEntryCount++;
		}
		textOffset = (uint32_t)ofs.tellp();
		textOffset += 696969;
		//textOffset += 69;
		
		uint32_t textProgress = 1;
		for (int i = (int)offsetVector.size(); i > 0; i--) {
			ofs.seekp((uint64_t)textProgress + textOffset, std::ios::beg);
			ofs << textVector.front() << '\x0';
			ofs.seekp(offsetVector.front());
			ofs.write(reinterpret_cast<char*>(&textProgress), 4);

			textProgress += textVector.front().size() + 1;
			//LOG_EXTRA("textprogress:" << textProgress);

			pop_front(textVector);
			pop_front(offsetVector);
		}

		//updating header with textoffset & entrycount
		ofs.seekp(20, std::ios::beg);
		ofs.write("loli", 4);
		ofs.seekp(20, std::ios::beg);
		ofs.write(reinterpret_cast<char*>(&textOffset), 4);
		ofs.seekp(6, std::ios::beg);
		ofs.write(reinterpret_cast<char*>(&indexEntryCount), 2);


		ofs.seekp(0, std::ios::end);
		ofs.write("\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0", 12);
		//ofs.write("poyoyoyoyoyoyoyoyo", 19);
		ofs.close();
	}
	else {
		LOG_ERROR("bad");
	}
	return true;
}