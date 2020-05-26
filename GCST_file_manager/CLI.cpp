#include "common.h"
#include "pack_unpack/ALAR.h"
#include "pack_unpack/ALTX.h"
#include "pack_unpack/general.h"
#include "pack_unpack/ALLZ.h"
#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>

/*
a wise man once said:

I find it weird you pass fs:path's everywhere. Ideally, the functions should be self-contained to only do what they are described to do.
So a decode or unpack function should probably just take a pointer to a file stream/buffer and export it to a dst or return that dst.
It doesn't make sense for the duty of a decode function to also read a file and export a file.
That should be the job of whatever called the decode or unpack function.
If you do insist on passing around fs::path objects everywhere, maybe instead do so by reference instead of copying them everywhere.

so fix that. some day.
*/


namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
	LOG_EXTRA("argc: " << argc);
	std::string gameDir = "tmp";
	std::string unpackDir = "tmp";
	LOG_MANDATORY("GCST file packer & unpacker");
	LOG_MANDATORY("program by Frain_Breeze, ALLZ decoder by Brolijah");
	//reading game & unpack dir from settings file
	if (fs::is_regular_file("settings.ini")) {
		std::ifstream settingsFile("settings.ini");
		if (!settingsFile.bad()) {
			std::string tmp;
			settingsFile >> tmp;
			if (tmp == "GAMEDIR:") {
				settingsFile >> gameDir;
			}
			settingsFile >> tmp;
			if (tmp == "UNPACKDIR:") {
				settingsFile >> unpackDir;
			}
			if ((gameDir == "none") || (unpackDir == "none")) {
				LOG_ERROR("one or both of the directories in the settings file are not set");
			}
			LOG_INFO("gameDir: " << gameDir << "  unpackDir: " << unpackDir);
		}
		settingsFile.close();
	}
	else {
		LOG_ERROR("the settings file could not be found, so one has been created");
		std::ofstream settingsFile("settings.ini");
		if (!settingsFile.bad()) {
			settingsFile << "GAMEDIR: none\n";
			settingsFile << "UNPACKDIR: none\n";
		}
		settingsFile.close();
		//return EXIT_FAILURE;
	}
	
	//actually processing commandline args
	if (argc == 1) { //no parameters given
		LOG_MANDATORY("No parameters were given, so here's the list of commands: (x) means x is optional." /*you can use gameDir and unpackDir instead of full directorynames."*/);
		LOG_MANDATORY("  unpack fileIn directoryOut");
		LOG_MANDATORY("  pack directoryIn fileOut");
		LOG_MANDATORY("  unpack_game  // unpacks gameDir to unpackDir (but filenames with spaces don't work, great");
		LOG_MANDATORY("     -unpack dirIn dirOut  // for if you want to not care about spaces")
		//LOG_MANDATORY("  pack_game // packs unpackDir to gameDir");
		LOG_MANDATORY("  ALAR_unpack fileIn directoryOut  // doesn't convert files within directory");
		LOG_MANDATORY("  ALAR_pack directoryIn fileOut (noConvert)  // by passing no-convert, it will skip files that aren't available in the format specified in PACKAGE_INFO.txt. be careful with this");
		LOG_MANDATORY("  ALTX_unpack fileIn fileOut format // output format is currently PAM, will change to TGA soon");
		//LOG_MANDATORY("  ALTX_pack fileIn fileOut  // input file can be PAM or TGA");
		LOG_MANDATORY("  ALSN_unpack fileIn fileOut  // saves header in filename.HEADER, for repacking later on");
		LOG_MANDATORY("  ALSN_pack fileIn fileOut  // shoves in anything you want, but just use .ogg (since that's what the game uses)");
		LOG_MANDATORY("  ALLZ_unpack fileIn (fileOut)  // if fileOut is not specified, it will overwrite the original file");
		//LOG_MANDATORY("  ALLZ_pack fileIn (fileOut)  // if fileOut is not specified, it will overwrite the original file");
		//LOG_MANDATORY(" Or you can just pass the program a file or folder, and the program will figure out what you want to do (most of the time)");
	}
	if (argc == 2) {
		std::string argComm = argv[1];
		if (argComm == "pack_game") {
			if (fs::is_directory(gameDir) && fs::is_directory(unpackDir)) {
				//pack_game()
			}
		}
		if (argComm == "unpack_game") {
			
			if (fs::is_directory(gameDir) && fs::is_directory(unpackDir)) {
				LOG_EXTRA("command: unpack game");
				unpack_game(gameDir, unpackDir);
			}
		}
		//LOG_WARN("not enough parameters supplied! supply no parameters to see list of commands");
	}
	if (argc > 2) {
		LOG_EXTRA("poyo");
		std::string argComm = argv[1];
		std::string arg1 = argv[2];
		std::string arg2 = "";
		std::string arg3 = "";
		if (argc == 4)
			arg2 = (std::string)argv[3];
		if (argc == 5)
			arg3 = (std::string)argv[4];
		LOG_EXTRA("argComm: " << argComm);

		if (argComm == "unpack_game") {
			LOG_EXTRA("command: unpack_game");
			if (argc == 4) {
				if (fs::is_directory(arg1) && fs::is_directory(arg2)) {
					unpack_game(arg1, arg2);
				}
				else {
					LOG_WARN("one of the args is incorrect, command \"unpack_game\" skipped");
				}
			}
		}

		if (argComm == "unpack") {
			LOG_EXTRA("command: unpack");
			if (argc == 4) {
				if (fs::is_regular_file(arg1) && fs::is_directory(arg2)) {
					unpack(arg1, arg2);
				}
				else {
					LOG_WARN("one of the args is incorrect, command \"unpack\" skipped");
				}
			}
		}

		if (argComm == "pack") {
			if (argc == 4) {
				if (fs::is_directory(arg1) && fs::is_regular_file(arg2)) {
					pack(arg1, arg2);
				}
				else {
					LOG_WARN("one of the args is incorrect, command \"pack\" skipped");
					LOG_WARN("args: " << arg1 << " " << arg2);
				}
			}
		}

		if (argComm == "ALLZ_unpack") {
			if (argc == 3) { //no fileOut, just overwrite the OG file
				if (fs::is_regular_file(arg1)) {
					ALLZ_decompress(arg1.c_str(), arg1.c_str());
				}
			}
			else if(argc == 4){ //fileOut has been supplied too
				if (fs::is_regular_file(arg1) && fs::is_regular_file(arg2)) {
					ALLZ_decompress(arg1.c_str(), arg2.c_str());
				}
			}
		}

		if (argComm == "ALSN_unpack") {
			LOG_EXTRA("command: ALSN unpack");
			if (argc == 4) {
				if (fs::is_regular_file(arg1) && fs::is_directory(arg2)) {
					unpack(arg1, arg2);
				}
				else {
					LOG_WARN("one of the args is incorrect, command \"unpack\" skipped");
				}
			}
		}

		if (argComm == "ALSN_pack") {
			LOG_EXTRA("command: ALSN pack");
			if (argc == 4) {
				if (fs::is_regular_file(arg1) && fs::is_directory(arg2)) {
					unpack(arg1, arg2);
				}
				else {
					LOG_WARN("one of the args is incorrect, command \"unpack\" skipped");
				}
			}
		}

		//and more
	}

	LOG_MANDATORY("done. successfully? who knows.");
	return EXIT_SUCCESS;
}