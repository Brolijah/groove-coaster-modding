#include "common.h"
#include "pack_unpack/ALAR.h"
#include "pack_unpack/ALTX.h"
#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
	LOG_MANDATORY("GCST file packer & unpacker");
	std::ifstream settingsFile("settings.ini");
	if (fs::is_regular_file(settingsFile)) {

	}

	return 0;
}